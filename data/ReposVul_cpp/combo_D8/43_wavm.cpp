#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

#include "Emscripten/Emscripten.h"
#include "IR/Module.h"
#include "IR/Operators.h"
#include "IR/Types.h"
#include "IR/Validate.h"
#include "IR/Value.h"
#include "Inline/BasicTypes.h"
#include "Inline/CLI.h"
#include "Inline/Errors.h"
#include "Inline/Hash.h"
#include "Inline/HashMap.h"
#include "Inline/Serialization.h"
#include "Inline/Timing.h"
#include "Logging/Logging.h"
#include "Runtime/Linker.h"
#include "Runtime/Runtime.h"
#include "ThreadTest/ThreadTest.h"
#include "WASTParse/WASTParse.h"

using namespace IR;
using namespace Runtime;

struct RootResolver : Resolver
{
	Compartment* getCompartment() const { return compartment; }
	HashMap<std::string, ModuleInstance*>& getModuleNameToInstanceMap() { return moduleNameToInstanceMap; }

	RootResolver(Compartment* inCompartment) : compartment(inCompartment) {}

	bool resolve(const std::string& moduleName,
				 const std::string& exportName,
				 ObjectType type,
				 Object*& outObject) override
	{
		auto namedInstance = getModuleNameToInstanceMap().get(moduleName);
		if(namedInstance)
		{
			outObject = getInstanceExport(*namedInstance, exportName);
			if(outObject)
			{
				if(isA(outObject, type)) { return true; }
				else
				{
					Log::printf(Log::error,
								"Resolved import %s.%s to a %s, but was expecting %s\n",
								moduleName.c_str(),
								exportName.c_str(),
								asString(getObjectType(outObject)).c_str(),
								asString(type).c_str());
					return false;
				}
			}
		}

		Log::printf(Log::error,
					"Generated stub for missing import %s.%s : %s\n",
					moduleName.c_str(),
					exportName.c_str(),
					asString(type).c_str());
		outObject = getStubObject(exportName, type);
		return true;
	}

	Object* getStubObject(const std::string& exportName, ObjectType type) const
	{
		switch(type.kind)
		{
		case IR::ObjectKind::function:
		{
			Serialization::ArrayOutputStream codeStream;
			OperatorEncoderStream encoder(codeStream);
			encoder.unreachable();
			encoder.end();

			IR::Module stubModule;
			DisassemblyNames stubModuleNames;
			stubModule.types.push_back(asFunctionType(type));
			stubModule.functions.defs.push_back({{0}, {}, std::move(codeStream.getBytes()), {}});
			stubModule.exports.push_back({"importStub", IR::ObjectKind::function, 0});
			stubModuleNames.functions.push_back({"importStub: " + exportName, {}, {}});
			IR::setDisassemblyNames(stubModule, stubModuleNames);
			IR::validateDefinitions(stubModule);

			auto stubModuleInstance
				= instantiateModule(getCompartment(), compileModule(stubModule), {}, "importStub");
			return getInstanceExport(stubModuleInstance, "importStub");
		}
		case IR::ObjectKind::memory:
		{
			return asObject(Runtime::createMemory(getCompartment(), asMemoryType(type)));
		}
		case IR::ObjectKind::table:
		{
			return asObject(Runtime::createTable(getCompartment(), asTableType(type)));
		}
		case IR::ObjectKind::global:
		{
			return asObject(Runtime::createGlobal(
				getCompartment(),
				asGlobalType(type),
				IR::Value(asGlobalType(type).valueType, IR::UntaggedValue())));
		}
		case IR::ObjectKind::exceptionType:
		{
			return asObject(
				Runtime::createExceptionTypeInstance(asExceptionType(type), "importStub"));
		}
		default: Errors::unreachable();
		};
	}

private:
	Compartment* compartment;
	HashMap<std::string, ModuleInstance*> moduleNameToInstanceMap;
};

struct CommandLineOptions
{
	const char* getFilename() const { return filename; }
	void setFilename(const char* value) { filename = value; }
	const char* getFunctionName() const { return functionName; }
	void setFunctionName(const char* value) { functionName = value; }
	char** getArgs() const { return args; }
	void setArgs(char** value) { args = value; }
	bool getOnlyCheck() const { return onlyCheck; }
	void setOnlyCheck(bool value) { onlyCheck = value; }
	bool getEnableEmscripten() const { return enableEmscripten; }
	void setEnableEmscripten(bool value) { enableEmscripten = value; }
	bool getEnableThreadTest() const { return enableThreadTest; }
	void setEnableThreadTest(bool value) { enableThreadTest = value; }
	bool getPrecompiled() const { return precompiled; }
	void setPrecompiled(bool value) { precompiled = value; }

private:
	const char* filename = nullptr;
	const char* functionName = nullptr;
	char** args = nullptr;
	bool onlyCheck = false;
	bool enableEmscripten = true;
	bool enableThreadTest = false;
	bool precompiled = false;
};

static int run(const CommandLineOptions& options)
{
	IR::Module irModule;

	if(!loadModule(options.getFilename(), irModule)) { return EXIT_FAILURE; }
	if(options.getOnlyCheck()) { return EXIT_SUCCESS; }

	Runtime::Module* module = nullptr;
	if(!options.getPrecompiled()) { module = Runtime::compileModule(irModule); }
	else
	{
		const UserSection* precompiledObjectSection = nullptr;
		for(const UserSection& userSection : irModule.userSections)
		{
			if(userSection.name == "wavm.precompiled_object")
			{
				precompiledObjectSection = &userSection;
				break;
			}
		}

		if(!precompiledObjectSection)
		{
			Log::printf(Log::error, "Input file did not contain 'wavm.precompiled_object' section");
			return EXIT_FAILURE;
		}
		else
		{
			module = Runtime::loadPrecompiledModule(irModule, precompiledObjectSection->data);
		}
	}

	Compartment* compartment = Runtime::createCompartment();
	Context* context = Runtime::createContext(compartment);
	RootResolver rootResolver(compartment);

	Emscripten::Instance* emscriptenInstance = nullptr;
	if(options.getEnableEmscripten())
	{
		emscriptenInstance = Emscripten::instantiate(compartment, irModule);
		if(emscriptenInstance)
		{
			rootResolver.getModuleNameToInstanceMap().set("env", emscriptenInstance->env);
			rootResolver.getModuleNameToInstanceMap().set("asm2wasm", emscriptenInstance->asm2wasm);
			rootResolver.getModuleNameToInstanceMap().set("global", emscriptenInstance->global);
		}
	}

	if(options.getEnableThreadTest())
	{
		ModuleInstance* threadTestInstance = ThreadTest::instantiate(compartment);
		rootResolver.getModuleNameToInstanceMap().set("threadTest", threadTestInstance);
	}

	LinkResult linkResult = linkModule(irModule, rootResolver);
	if(!linkResult.success)
	{
		Log::printf(Log::error, "Failed to link module:\n");
		for(auto& missingImport : linkResult.missingImports)
		{
			Log::printf(Log::error,
						"Missing import: module=\"%s\" export=\"%s\" type=\"%s\"\n",
						missingImport.moduleName.c_str(),
						missingImport.exportName.c_str(),
						asString(missingImport.type).c_str());
		}
		return EXIT_FAILURE;
	}

	ModuleInstance* moduleInstance = instantiateModule(
		compartment, module, std::move(linkResult.resolvedImports), options.getFilename());
	if(!moduleInstance) { return EXIT_FAILURE; }

	FunctionInstance* startFunction = getStartFunction(moduleInstance);
	if(startFunction) { invokeFunctionChecked(context, startFunction, {}); }

	if(options.getEnableEmscripten())
	{
		Emscripten::initializeGlobals(context, irModule, moduleInstance);
	}

	FunctionInstance* functionInstance;
	if(!options.getFunctionName())
	{
		functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "main"));
		if(!functionInstance)
		{ functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "_main")); }
		if(!functionInstance)
		{
			Log::printf(Log::error, "Module does not export main function\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		functionInstance
			= asFunctionNullable(getInstanceExport(moduleInstance, options.getFunctionName()));
		if(!functionInstance)
		{
			Log::printf(Log::error, "Module does not export '%s'\n", options.getFunctionName());
			return EXIT_FAILURE;
		}
	}
	FunctionType functionType = getFunctionType(functionInstance);

	std::vector<Value> invokeArgs;
	if(!options.getFunctionName())
	{
		if(functionType.params().size() == 2)
		{
			if(!emscriptenInstance)
			{
				Log::printf(
					Log::error,
					"Module does not declare a default memory object to put arguments in.\n");
				return EXIT_FAILURE;
			}
			else
			{
				std::vector<const char*> argStrings;
				argStrings.push_back(options.getFilename());
				char** args = options.getArgs();
				while(*args) { argStrings.push_back(*args++); };

				wavmAssert(emscriptenInstance);
				Emscripten::injectCommandArgs(emscriptenInstance, argStrings, invokeArgs);
			}
		}
		else if(functionType.params().size() > 0)
		{
			Log::printf(Log::error,
						"WebAssembly function requires %" PRIu64
						" argument(s), but only 0 or 2 can be passed!",
						functionType.params().size());
			return EXIT_FAILURE;
		}
	}
	else
	{
		for(U32 i = 0; options.getArgs()[i]; ++i)
		{
			Value value;
			switch(functionType.params()[i])
			{
			case ValueType::i32: value = (U32)atoi(options.getArgs()[i]); break;
			case ValueType::i64: value = (U64)atol(options.getArgs()[i]); break;
			case ValueType::f32: value = (F32)atof(options.getArgs()[i]); break;
			case ValueType::f64: value = atof(options.getArgs()[i]); break;
			case ValueType::v128:
			case ValueType::anyref:
			case ValueType::anyfunc:
				Errors::fatalf("Cannot parse command-line argument for %s function parameter",
							   asString(functionType.params()[i]));
			default: Errors::unreachable();
			}
			invokeArgs.push_back(value);
		}
	}

	Timing::Timer executionTimer;
	IR::ValueTuple functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);
	Timing::logTimer("Invoked function", executionTimer);

	if(options.getFunctionName())
	{
		Log::printf(Log::debug,
					"%s returned: %s\n",
					options.getFunctionName(),
					asString(functionResults).c_str());
		return EXIT_SUCCESS;
	}
	else if(functionResults.size() == 1 && functionResults[0].type == ValueType::i32)
	{
		return functionResults[0].i32;
	}
	else
	{
		return EXIT_SUCCESS;
	}
}

static void showHelp()
{
	Log::printf(Log::error,
				"Usage: wavm [switches] [programfile] [--] [arguments]\n"
				"  in.wast|in.wasm       Specify program file (.wast/.wasm)\n"
				"  -c|--check            Exit after checking that the program is valid\n"
				"  -d|--debug            Write additional debug information to stdout\n"
				"  -f|--function name    Specify function name to run in module rather than main\n"
				"  -h|--help             Display this message\n"
				"  --disable-emscripten  Disable Emscripten intrinsics\n"
				"  --enable-thread-test  Enable ThreadTest intrinsics\n"
				"  --precompiled         Use precompiled object code in programfile\n"
				"  --                    Stop parsing arguments\n");
}

int main(int argc, char** argv)
{
	CommandLineOptions options;
	options.setArgs(argv);
	while(*++options.getArgs())
	{
		if(!strcmp(*options.getArgs(), "--function") || !strcmp(*options.getArgs(), "-f"))
		{
			if(!*++options.getArgs())
			{
				showHelp();
				return EXIT_FAILURE;
			}
			options.setFunctionName(*options.getArgs());
		}
		else if(!strcmp(*options.getArgs(), "--check") || !strcmp(*options.getArgs(), "-c"))
		{
			options.setOnlyCheck(true);
		}
		else if(!strcmp(*options.getArgs(), "--debug") || !strcmp(*options.getArgs(), "-d"))
		{
			Log::setCategoryEnabled(Log::debug, true);
		}
		else if(!strcmp(*options.getArgs(), "--disable-emscripten"))
		{
			options.setEnableEmscripten(false);
		}
		else if(!strcmp(*options.getArgs(), "--enable-thread-test"))
		{
			options.setEnableThreadTest(true);
		}
		else if(!strcmp(*options.getArgs(), "--precompiled"))
		{
			options.setPrecompiled(true);
		}
		else if(!strcmp(*options.getArgs(), "--"))
		{
			++options.getArgs();
			break;
		}
		else if(!strcmp(*options.getArgs(), "--help") || !strcmp(*options.getArgs(), "-h"))
		{
			showHelp();
			return EXIT_SUCCESS;
		}
		else if(!options.getFilename())
		{
			options.setFilename(*options.getArgs());
		}
		else
		{
			break;
		}
	}

	if(!options.getFilename())
	{
		showHelp();
		return EXIT_FAILURE;
	}

	Runtime::setUnhandledExceptionHandler([](Runtime::Exception&& exception) {
		Errors::fatalf("Runtime exception: %s\n", describeException(exception).c_str());
	});

	return run(options);
}