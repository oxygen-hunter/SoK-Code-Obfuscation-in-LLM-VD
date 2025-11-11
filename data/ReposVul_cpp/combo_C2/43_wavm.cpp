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
	Compartment* compartment;
	HashMap<std::string, ModuleInstance*> moduleNameToInstanceMap;

	RootResolver(Compartment* inCompartment) : compartment(inCompartment) {}

	bool resolve(const std::string& moduleName,
				 const std::string& exportName,
				 ObjectType type,
				 Object*& outObject) override
	{
		int dispatch = 0;
		while (true) {
			switch (dispatch) {
				case 0: {
					auto namedInstance = moduleNameToInstanceMap.get(moduleName);
					if(namedInstance) {
						outObject = getInstanceExport(*namedInstance, exportName);
						dispatch = 1;
					} else {
						dispatch = 3;
					}
					break;
				}
				case 1: {
					if(outObject) {
						if(isA(outObject, type)) { return true; }
						else {
							dispatch = 2;
						}
					} else {
						dispatch = 3;
					}
					break;
				}
				case 2: {
					Log::printf(Log::error,
								"Resolved import %s.%s to a %s, but was expecting %s\n",
								moduleName.c_str(),
								exportName.c_str(),
								asString(getObjectType(outObject)).c_str(),
								asString(type).c_str());
					return false;
				}
				case 3: {
					Log::printf(Log::error,
								"Generated stub for missing import %s.%s : %s\n",
								moduleName.c_str(),
								exportName.c_str(),
								asString(type).c_str());
					outObject = getStubObject(exportName, type);
					return true;
				}
			}
		}
	}

	Object* getStubObject(const std::string& exportName, ObjectType type) const
	{
		int dispatch = 0;
		while (true) {
			switch(dispatch) {
				case 0: {
					switch(type.kind) {
						case IR::ObjectKind::function: dispatch = 1; break;
						case IR::ObjectKind::memory: dispatch = 2; break;
						case IR::ObjectKind::table: dispatch = 3; break;
						case IR::ObjectKind::global: dispatch = 4; break;
						case IR::ObjectKind::exceptionType: dispatch = 5; break;
						default: Errors::unreachable();
					}
					break;
				}
				case 1: {
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
						= instantiateModule(compartment, compileModule(stubModule), {}, "importStub");
					return getInstanceExport(stubModuleInstance, "importStub");
				}
				case 2: {
					return asObject(Runtime::createMemory(compartment, asMemoryType(type)));
				}
				case 3: {
					return asObject(Runtime::createTable(compartment, asTableType(type)));
				}
				case 4: {
					return asObject(Runtime::createGlobal(
						compartment,
						asGlobalType(type),
						IR::Value(asGlobalType(type).valueType, IR::UntaggedValue())));
				}
				case 5: {
					return asObject(
						Runtime::createExceptionTypeInstance(asExceptionType(type), "importStub"));
				}
			}
		}
	}
};

struct CommandLineOptions
{
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
	int dispatch = 0;
	IR::Module irModule;
	Runtime::Module* module = nullptr;
	Compartment* compartment = nullptr;
	Context* context = nullptr;
	RootResolver rootResolver(nullptr);
	Emscripten::Instance* emscriptenInstance = nullptr;
	LinkResult linkResult;
	ModuleInstance* moduleInstance = nullptr;
	FunctionInstance* startFunction = nullptr;
	FunctionInstance* functionInstance = nullptr;
	FunctionType functionType;
	std::vector<Value> invokeArgs;
	Timing::Timer executionTimer;
	IR::ValueTuple functionResults;

	while (true) {
		switch (dispatch) {
			case 0: {
				if(!loadModule(options.filename, irModule)) { return EXIT_FAILURE; }
				if(options.onlyCheck) { return EXIT_SUCCESS; }
				dispatch = 1;
				break;
			}
			case 1: {
				if(!options.precompiled) {
					module = Runtime::compileModule(irModule);
					dispatch = 3;
				} else {
					dispatch = 2;
				}
				break;
			}
			case 2: {
				const UserSection* precompiledObjectSection = nullptr;
				for(const UserSection& userSection : irModule.userSections) {
					if(userSection.name == "wavm.precompiled_object") {
						precompiledObjectSection = &userSection;
						break;
					}
				}

				if(!precompiledObjectSection) {
					Log::printf(Log::error, "Input file did not contain 'wavm.precompiled_object' section");
					return EXIT_FAILURE;
				} else {
					module = Runtime::loadPrecompiledModule(irModule, precompiledObjectSection->data);
					dispatch = 3;
				}
				break;
			}
			case 3: {
				compartment = Runtime::createCompartment();
				context = Runtime::createContext(compartment);
				rootResolver = RootResolver(compartment);

				if(options.enableEmscripten) {
					emscriptenInstance = Emscripten::instantiate(compartment, irModule);
					if(emscriptenInstance) {
						rootResolver.moduleNameToInstanceMap.set("env", emscriptenInstance->env);
						rootResolver.moduleNameToInstanceMap.set("asm2wasm", emscriptenInstance->asm2wasm);
						rootResolver.moduleNameToInstanceMap.set("global", emscriptenInstance->global);
					}
				}

				if(options.enableThreadTest) {
					ModuleInstance* threadTestInstance = ThreadTest::instantiate(compartment);
					rootResolver.moduleNameToInstanceMap.set("threadTest", threadTestInstance);
				}

				linkResult = linkModule(irModule, rootResolver);
				if(!linkResult.success) {
					Log::printf(Log::error, "Failed to link module:\n");
					for(auto& missingImport : linkResult.missingImports) {
						Log::printf(Log::error,
									"Missing import: module=\"%s\" export=\"%s\" type=\"%s\"\n",
									missingImport.moduleName.c_str(),
									missingImport.exportName.c_str(),
									asString(missingImport.type).c_str());
					}
					return EXIT_FAILURE;
				}
				dispatch = 4;
				break;
			}
			case 4: {
				moduleInstance = instantiateModule(
					compartment, module, std::move(linkResult.resolvedImports), options.filename);
				if(!moduleInstance) { return EXIT_FAILURE; }
				startFunction = getStartFunction(moduleInstance);
				if(startFunction) { invokeFunctionChecked(context, startFunction, {}); }
				dispatch = 5;
				break;
			}
			case 5: {
				if(options.enableEmscripten) {
					Emscripten::initializeGlobals(context, irModule, moduleInstance);
				}
				if(!options.functionName) {
					functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "main"));
					if(!functionInstance) {
						functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, "_main"));
					}
					if(!functionInstance) {
						Log::printf(Log::error, "Module does not export main function\n");
						return EXIT_FAILURE;
					}
				} else {
					functionInstance = asFunctionNullable(getInstanceExport(moduleInstance, options.functionName));
					if(!functionInstance) {
						Log::printf(Log::error, "Module does not export '%s'\n", options.functionName);
						return EXIT_FAILURE;
					}
				}
				functionType = getFunctionType(functionInstance);
				dispatch = 6;
				break;
			}
			case 6: {
				if(!options.functionName) {
					if(functionType.params().size() == 2) {
						if(!emscriptenInstance) {
							Log::printf(
								Log::error,
								"Module does not declare a default memory object to put arguments in.\n");
							return EXIT_FAILURE;
						} else {
							std::vector<const char*> argStrings;
							argStrings.push_back(options.filename);
							char** args = options.args;
							while(*args) { argStrings.push_back(*args++); };

							wavmAssert(emscriptenInstance);
							Emscripten::injectCommandArgs(emscriptenInstance, argStrings, invokeArgs);
						}
					} else if(functionType.params().size() > 0) {
						Log::printf(Log::error,
									"WebAssembly function requires %" PRIu64
									" argument(s), but only 0 or 2 can be passed!",
									functionType.params().size());
						return EXIT_FAILURE;
					}
				} else {
					for(U32 i = 0; options.args[i]; ++i) {
						Value value;
						switch(functionType.params()[i]) {
							case ValueType::i32: value = (U32)atoi(options.args[i]); break;
							case ValueType::i64: value = (U64)atol(options.args[i]); break;
							case ValueType::f32: value = (F32)atof(options.args[i]); break;
							case ValueType::f64: value = atof(options.args[i]); break;
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
				dispatch = 7;
				break;
			}
			case 7: {
				functionResults = invokeFunctionChecked(context, functionInstance, invokeArgs);
				Timing::logTimer("Invoked function", executionTimer);

				if(options.functionName) {
					Log::printf(Log::debug,
								"%s returned: %s\n",
								options.functionName,
								asString(functionResults).c_str());
					return EXIT_SUCCESS;
				} else if(functionResults.size() == 1 && functionResults[0].type == ValueType::i32) {
					return functionResults[0].i32;
				} else {
					return EXIT_SUCCESS;
				}
			}
		}
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
	options.args = argv;
	int dispatch = 0;

	while (true) {
		switch (dispatch) {
			case 0: {
				if(!*++options.args) {
					dispatch = 4;
				} else if(!strcmp(*options.args, "--function") || !strcmp(*options.args, "-f")) {
					dispatch = 1;
				} else if(!strcmp(*options.args, "--check") || !strcmp(*options.args, "-c")) {
					options.onlyCheck = true;
				} else if(!strcmp(*options.args, "--debug") || !strcmp(*options.args, "-d")) {
					Log::setCategoryEnabled(Log::debug, true);
				} else if(!strcmp(*options.args, "--disable-emscripten")) {
					options.enableEmscripten = false;
				} else if(!strcmp(*options.args, "--enable-thread-test")) {
					options.enableThreadTest = true;
				} else if(!strcmp(*options.args, "--precompiled")) {
					options.precompiled = true;
				} else if(!strcmp(*options.args, "--")) {
					++options.args;
					dispatch = 5;
				} else if(!strcmp(*options.args, "--help") || !strcmp(*options.args, "-h")) {
					showHelp();
					return EXIT_SUCCESS;
				} else if(!options.filename) {
					options.filename = *options.args;
				} else {
					break;
				}
				break;
			}
			case 1: {
				if(!*++options.args) {
					showHelp();
					return EXIT_FAILURE;
				}
				options.functionName = *options.args;
				dispatch = 0;
				break;
			}
			case 2: {
				dispatch = 0;
				break;
			}
			case 3: {
				break;
			}
			case 4: {
				showHelp();
				return EXIT_FAILURE;
			}
			case 5: {
				break;
			}
		}
	}

	if(!options.filename) {
		showHelp();
		return EXIT_FAILURE;
	}

	Runtime::setUnhandledExceptionHandler([](Runtime::Exception&& exception) {
		Errors::fatalf("Runtime exception: %s\n", describeException(exception).c_str());
	});

	return run(options);
}