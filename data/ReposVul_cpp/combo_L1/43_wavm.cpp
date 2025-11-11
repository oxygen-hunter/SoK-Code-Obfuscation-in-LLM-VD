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

struct OX7B4DF339 : Resolver
{
	Compartment* OX1F2D9A7C;
	HashMap<std::string, ModuleInstance*> OX5E6B7A2F;

	OX7B4DF339(Compartment* OX7C9E1F3B) : OX1F2D9A7C(OX7C9E1F3B) {}

	bool OX2E3A4B8C(const std::string& OX9A3D4F6E,
				 const std::string& OX4C7B8A1D,
				 ObjectType OX6F1A2D8B,
				 Object*& OX8B9C1D7E) override
	{
		auto OX3D7E1F6A = OX5E6B7A2F.get(OX9A3D4F6E);
		if(OX3D7E1F6A)
		{
			OX8B9C1D7E = getInstanceExport(*OX3D7E1F6A, OX4C7B8A1D);
			if(OX8B9C1D7E)
			{
				if(isA(OX8B9C1D7E, OX6F1A2D8B)) { return true; }
				else
				{
					Log::printf(Log::error,
								"Resolved import %s.%s to a %s, but was expecting %s\n",
								OX9A3D4F6E.c_str(),
								OX4C7B8A1D.c_str(),
								asString(getObjectType(OX8B9C1D7E)).c_str(),
								asString(OX6F1A2D8B).c_str());
					return false;
				}
			}
		}

		Log::printf(Log::error,
					"Generated stub for missing import %s.%s : %s\n",
					OX9A3D4F6E.c_str(),
					OX4C7B8A1D.c_str(),
					asString(OX6F1A2D8B).c_str());
		OX8B9C1D7E = OX3F2A1C9B(OX4C7B8A1D, OX6F1A2D8B);
		return true;
	}

	Object* OX3F2A1C9B(const std::string& OX4C7B8A1D, ObjectType OX6F1A2D8B) const
	{
		switch(OX6F1A2D8B.kind)
		{
		case IR::ObjectKind::function:
		{
			Serialization::ArrayOutputStream OX9C4A7F2B;
			OperatorEncoderStream OX5B3E1D9F(OX9C4A7F2B);
			OX5B3E1D9F.unreachable();
			OX5B3E1D9F.end();

			IR::Module OX2D9A4C7B;
			DisassemblyNames OX7E1F5B3C;
			OX2D9A4C7B.types.push_back(asFunctionType(OX6F1A2D8B));
			OX2D9A4C7B.functions.defs.push_back({{0}, {}, std::move(OX9C4A7F2B.getBytes()), {}});
			OX2D9A4C7B.exports.push_back({"importStub", IR::ObjectKind::function, 0});
			OX7E1F5B3C.functions.push_back({"importStub: " + OX4C7B8A1D, {}, {}});
			IR::setDisassemblyNames(OX2D9A4C7B, OX7E1F5B3C);
			IR::validateDefinitions(OX2D9A4C7B);

			auto OX1A2D9C7E
				= instantiateModule(OX1F2D9A7C, compileModule(OX2D9A4C7B), {}, "importStub");
			return getInstanceExport(OX1A2D9C7E, "importStub");
		}
		case IR::ObjectKind::memory:
		{
			return asObject(Runtime::createMemory(OX1F2D9A7C, asMemoryType(OX6F1A2D8B)));
		}
		case IR::ObjectKind::table:
		{
			return asObject(Runtime::createTable(OX1F2D9A7C, asTableType(OX6F1A2D8B)));
		}
		case IR::ObjectKind::global:
		{
			return asObject(Runtime::createGlobal(
				OX1F2D9A7C,
				asGlobalType(OX6F1A2D8B),
				IR::Value(asGlobalType(OX6F1A2D8B).valueType, IR::UntaggedValue())));
		}
		case IR::ObjectKind::exceptionType:
		{
			return asObject(
				Runtime::createExceptionTypeInstance(asExceptionType(OX6F1A2D8B), "importStub"));
		}
		default: Errors::unreachable();
		};
	}
};

struct OX4F1B2C9D
{
	const char* OX3A7E4F2B = nullptr;
	const char* OX8D9F1C6A = nullptr;
	char** OX5C9A7B3E = nullptr;
	bool OX9E1F4A6B = false;
	bool OX2B3E7C9D = true;
	bool OX6F8A1D4C = false;
	bool OX7D2B9A4E = false;
};

static int OX7A9D3F2C(const OX4F1B2C9D& OX6B2A4D9F)
{
	IR::Module OX1F9C7B2A;

	if(!loadModule(OX6B2A4D9F.OX3A7E4F2B, OX1F9C7B2A)) { return EXIT_FAILURE; }
	if(OX6B2A4D9F.OX9E1F4A6B) { return EXIT_SUCCESS; }

	Runtime::Module* OX3D7F9C4A = nullptr;
	if(!OX6B2A4D9F.OX7D2B9A4E) { OX3D7F9C4A = Runtime::compileModule(OX1F9C7B2A); }
	else
	{
		const UserSection* OX5A3B8E1C = nullptr;
		for(const UserSection& OX9F2B7C8D : OX1F9C7B2A.userSections)
		{
			if(OX9F2B7C8D.name == "wavm.precompiled_object")
			{
				OX5A3B8E1C = &OX9F2B7C8D;
				break;
			}
		}

		if(!OX5A3B8E1C)
		{
			Log::printf(Log::error, "Input file did not contain 'wavm.precompiled_object' section");
			return EXIT_FAILURE;
		}
		else
		{
			OX3D7F9C4A = Runtime::loadPrecompiledModule(OX1F9C7B2A, OX5A3B8E1C->data);
		}
	}

	Compartment* OX8B1D4F2A = Runtime::createCompartment();
	Context* OX7C9E1F3B = Runtime::createContext(OX8B1D4F2A);
	OX7B4DF339 OX3A6B9E4D(OX8B1D4F2A);

	Emscripten::Instance* OX2F9A3D7C = nullptr;
	if(OX6B2A4D9F.OX2B3E7C9D)
	{
		OX2F9A3D7C = Emscripten::instantiate(OX8B1D4F2A, OX1F9C7B2A);
		if(OX2F9A3D7C)
		{
			OX3A6B9E4D.OX5E6B7A2F.set("env", OX2F9A3D7C->env);
			OX3A6B9E4D.OX5E6B7A2F.set("asm2wasm", OX2F9A3D7C->asm2wasm);
			OX3A6B9E4D.OX5E6B7A2F.set("global", OX2F9A3D7C->global);
		}
	}

	if(OX6B2A4D9F.OX6F8A1D4C)
	{
		ModuleInstance* OX9D3E1F7B = ThreadTest::instantiate(OX8B1D4F2A);
		OX3A6B9E4D.OX5E6B7A2F.set("threadTest", OX9D3E1F7B);
	}

	LinkResult OX5B3E7A1C = linkModule(OX1F9C7B2A, OX3A6B9E4D);
	if(!OX5B3E7A1C.success)
	{
		Log::printf(Log::error, "Failed to link module:\n");
		for(auto& OX7A2D8F1C : OX5B3E7A1C.missingImports)
		{
			Log::printf(Log::error,
						"Missing import: module=\"%s\" export=\"%s\" type=\"%s\"\n",
						OX7A2D8F1C.moduleName.c_str(),
						OX7A2D8F1C.exportName.c_str(),
						asString(OX7A2D8F1C.type).c_str());
		}
		return EXIT_FAILURE;
	}

	ModuleInstance* OX3C7B9A4D = instantiateModule(
		OX8B1D4F2A, OX3D7F9C4A, std::move(OX5B3E7A1C.resolvedImports), OX6B2A4D9F.OX3A7E4F2B);
	if(!OX3C7B9A4D) { return EXIT_FAILURE; }

	FunctionInstance* OX4A7C1B9E = getStartFunction(OX3C7B9A4D);
	if(OX4A7C1B9E) { invokeFunctionChecked(OX7C9E1F3B, OX4A7C1B9E, {}); }

	if(OX6B2A4D9F.OX2B3E7C9D)
	{
		Emscripten::initializeGlobals(OX7C9E1F3B, OX1F9C7B2A, OX3C7B9A4D);
	}

	FunctionInstance* OX9C1F3A7E;
	if(!OX6B2A4D9F.OX8D9F1C6A)
	{
		OX9C1F3A7E = asFunctionNullable(getInstanceExport(OX3C7B9A4D, "main"));
		if(!OX9C1F3A7E)
		{ OX9C1F3A7E = asFunctionNullable(getInstanceExport(OX3C7B9A4D, "_main")); }
		if(!OX9C1F3A7E)
		{
			Log::printf(Log::error, "Module does not export main function\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		OX9C1F3A7E
			= asFunctionNullable(getInstanceExport(OX3C7B9A4D, OX6B2A4D9F.OX8D9F1C6A));
		if(!OX9C1F3A7E)
		{
			Log::printf(Log::error, "Module does not export '%s'\n", OX6B2A4D9F.OX8D9F1C6A);
			return EXIT_FAILURE;
		}
	}
	FunctionType OX8B3D7F1A = getFunctionType(OX9C1F3A7E);

	std::vector<Value> OX1C9E5A3B;
	if(!OX6B2A4D9F.OX8D9F1C6A)
	{
		if(OX8B3D7F1A.params().size() == 2)
		{
			if(!OX2F9A3D7C)
			{
				Log::printf(
					Log::error,
					"Module does not declare a default memory object to put arguments in.\n");
				return EXIT_FAILURE;
			}
			else
			{
				std::vector<const char*> OX5F1A7D3B;
				OX5F1A7D3B.push_back(OX6B2A4D9F.OX3A7E4F2B);
				char** OX6E9F2B4C = OX6B2A4D9F.OX5C9A7B3E;
				while(*OX6E9F2B4C) { OX5F1A7D3B.push_back(*OX6E9F2B4C++); };

				wavmAssert(OX2F9A3D7C);
				Emscripten::injectCommandArgs(OX2F9A3D7C, OX5F1A7D3B, OX1C9E5A3B);
			}
		}
		else if(OX8B3D7F1A.params().size() > 0)
		{
			Log::printf(Log::error,
						"WebAssembly function requires %" PRIu64
						" argument(s), but only 0 or 2 can be passed!",
						OX8B3D7F1A.params().size());
			return EXIT_FAILURE;
		}
	}
	else
	{
		for(U32 OX7F3C1A9B = 0; OX6B2A4D9F.OX5C9A7B3E[OX7F3C1A9B]; ++OX7F3C1A9B)
		{
			Value OX2D9F8A7C;
			switch(OX8B3D7F1A.params()[OX7F3C1A9B])
			{
			case ValueType::i32: OX2D9F8A7C = (U32)atoi(OX6B2A4D9F.OX5C9A7B3E[OX7F3C1A9B]); break;
			case ValueType::i64: OX2D9F8A7C = (U64)atol(OX6B2A4D9F.OX5C9A7B3E[OX7F3C1A9B]); break;
			case ValueType::f32: OX2D9F8A7C = (F32)atof(OX6B2A4D9F.OX5C9A7B3E[OX7F3C1A9B]); break;
			case ValueType::f64: OX2D9F8A7C = atof(OX6B2A4D9F.OX5C9A7B3E[OX7F3C1A9B]); break;
			case ValueType::v128:
			case ValueType::anyref:
			case ValueType::anyfunc:
				Errors::fatalf("Cannot parse command-line argument for %s function parameter",
							   asString(OX8B3D7F1A.params()[OX7F3C1A9B]));
			default: Errors::unreachable();
			}
			OX1C9E5A3B.push_back(OX2D9F8A7C);
		}
	}

	Timing::Timer OX3A4D7F9E;
	IR::ValueTuple OX9A1C5B3E = invokeFunctionChecked(OX7C9E1F3B, OX9C1F3A7E, OX1C9E5A3B);
	Timing::logTimer("Invoked function", OX3A4D7F9E);

	if(OX6B2A4D9F.OX8D9F1C6A)
	{
		Log::printf(Log::debug,
					"%s returned: %s\n",
					OX6B2A4D9F.OX8D9F1C6A,
					asString(OX9A1C5B3E).c_str());
		return EXIT_SUCCESS;
	}
	else if(OX9A1C5B3E.size() == 1 && OX9A1C5B3E[0].type == ValueType::i32)
	{
		return OX9A1C5B3E[0].i32;
	}
	else
	{
		return EXIT_SUCCESS;
	}
}

static void OX2B4F1A9E()
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

int main(int OX7F1A6D3C, char** OX9C5B3E1F)
{
	OX4F1B2C9D OX6B2A4D9F;
	OX6B2A4D9F.OX5C9A7B3E = OX9C5B3E1F;
	while(*++OX6B2A4D9F.OX5C9A7B3E)
	{
		if(!strcmp(*OX6B2A4D9F.OX5C9A7B3E, "--function") || !strcmp(*OX6B2A4D9F.OX5C9A7B3E, "-f"))
		{
			if(!*++OX6B2A4D9F.OX5C9A7B3E)
			{
				OX2B4F1A9E();
				return EXIT_FAILURE;
			}
			OX6B2A4D9F.OX8D9F1C6A = *OX6B2A4D9F.OX5C9A7B3E;
		}
		else if(!strcmp(*OX6B2A4D9F.OX5C9A7B3E, "--check") || !strcmp(*OX6B2A4D9F.OX5C9A7B3E, "-c"))
		{
			OX6B2A4D9F.OX9E1F4A6B = true;
		}
		else if(!strcmp(*OX6B2A4D9F.OX5C9A7B3E, "--debug") || !strcmp(*OX6B2A4D9F.OX5C9A7B3E, "-d"))
		{
			Log::setCategoryEnabled(Log::debug, true);
		}
		else if(!strcmp(*OX6B2A4D9F.OX5C9A7B3E, "--disable-emscripten"))
		{
			OX6B2A4D9F.OX2B3E7C9D = false;
		}
		else if(!strcmp(*OX6B2A4D9F.OX5C9A7B3E, "--enable-thread-test"))
		{
			OX6B2A4D9F.OX6F8A1D4C = true;
		}
		else if(!strcmp(*OX6B2A4D9F.OX5C9A7B3E, "--precompiled"))
		{
			OX6B2A4D9F.OX7D2B9A4E = true;
		}
		else if(!strcmp(*OX6B2A4D9F.OX5C9A7B3E, "--"))
		{
			++OX6B2A4D9F.OX5C9A7B3E;
			break;
		}
		else if(!strcmp(*OX6B2A4D9F.OX5C9A7B3E, "--help") || !strcmp(*OX6B2A4D9F.OX5C9A7B3E, "-h"))
		{
			OX2B4F1A9E();
			return EXIT_SUCCESS;
		}
		else if(!OX6B2A4D9F.OX3A7E4F2B)
		{
			OX6B2A4D9F.OX3A7E4F2B = *OX6B2A4D9F.OX5C9A7B3E;
		}
		else
		{
			break;
		}
	}

	if(!OX6B2A4D9F.OX3A7E4F2B)
	{
		OX2B4F1A9E();
		return EXIT_FAILURE;
	}

	Runtime::setUnhandledExceptionHandler([](Runtime::Exception&& OX8B7C1D4F) {
		Errors::fatalf("Runtime exception: %s\n", describeException(OX8B7C1D4F).c_str());
	});

	return OX7A9D3F2C(OX6B2A4D9F);
}