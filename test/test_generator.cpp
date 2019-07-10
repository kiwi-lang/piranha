#include <pch.h>

#include "test_generator.h"

#include "../include/float_cast_node.h"
#include "../include/string_cast_node.h"
#include "../include/literal_node.h"
#include "../include/float_conversions.h"
#include "../include/string_conversions.h"
#include "../include/fundamental_types.h"
#include "../include/default_literal_node.h"

TestGenerator::TestGenerator() {
	/* void */
}

TestGenerator::~TestGenerator() {
	/* void */
}

void TestGenerator::registerBuiltinNodeTypes() {
	// Builtin Types
	registerBuiltinType<piranha::FloatCastNode>("__piranha__float");
	registerBuiltinType<piranha::StringCastNode>("__piranha__string");

	// Literals
	registerLiteralType<piranha::DefaultLiteralStringNode, piranha::LiteralStringType>();
	registerLiteralType<piranha::DefaultLiteralIntNode, piranha::LiteralIntType>();
	registerLiteralType<piranha::DefaultLiteralFloatNode, piranha::LiteralFloatType>();
	registerLiteralType<piranha::DefaultLiteralBoolNode, piranha::LiteralBoolType>();

	// Conversions
	registerConversion<piranha::StringToFloatConversionNode>(
		{&piranha::FundamentalType::StringType, &piranha::FundamentalType::FloatType }
	);
	registerConversion<piranha::FloatToStringConversionNode>(
		{ &piranha::FundamentalType::FloatType, &piranha::FundamentalType::StringType }
	);
}
