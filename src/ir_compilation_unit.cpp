#include "../include/ir_compilation_unit.h"

#include "../include/scanner.h"
#include "../include/ir_node.h"
#include "../include/ir_attribute_list.h"
#include "../include/ir_attribute.h"
#include "../include/error_list.h"
#include "../include/node_program.h"
#include "../include/language_rules.h"

#include <cctype>
#include <fstream>
#include <cassert>
#include <sstream>

piranha::IrCompilationUnit::~IrCompilationUnit() {
    delete m_scanner;
    m_scanner = nullptr;

    delete m_parser;
    m_parser = nullptr;
}

void piranha::IrCompilationUnit::build(NodeProgram *program) {
    int nodeCount = getNodeCount();
    for (int i = 0; i < nodeCount; i++) {
        m_nodes[i]->generateNode(nullptr, program);
    }
}

piranha::IrCompilationUnit::ParseResult piranha::IrCompilationUnit::parseFile(
    const Path &filename, IrCompilationUnit *topLevel) 
{
    m_path = filename;
    
    std::ifstream inFile(filename.toString());
    if (!inFile.good()) {
        return IO_ERROR;
    }

    return parseHelper(inFile);
}

piranha::IrCompilationUnit::ParseResult piranha::IrCompilationUnit::parse(
    const char *sdl, IrCompilationUnit *topLevel) 
{
    std::istringstream sdlStream(sdl);
    return parse(sdlStream);
}

piranha::IrCompilationUnit::ParseResult piranha::IrCompilationUnit::parse(
    std::istream &stream, IrCompilationUnit *topLevel)
{
    if (!stream.good() && stream.eof()) {
        return IO_ERROR;
    }

    return parseHelper(stream);
}

void piranha::IrCompilationUnit::_checkInstantiation() {
    IrContextTree *mainContext = new IrContextTree(nullptr, true);

    int componentCount = getComponentCount();
    for (int i = 0; i < componentCount; i++) {
        m_components[i]->checkReferences(mainContext);
    }
}

void piranha::IrCompilationUnit::_checkTypes() {
    IrContextTree *mainContext = new IrContextTree(nullptr, true);

    int componentCount = getComponentCount();
    for (int i = 0; i < componentCount; i++) {
        m_components[i]->checkTypes(mainContext);
    }
}

void piranha::IrCompilationUnit::_expand() {
    IrContextTree *mainContext = new IrContextTree(nullptr, true);

    int componentCount = getComponentCount();
    for (int i = 0; i < componentCount; i++) {
        m_components[i]->expand(mainContext);
    }
}

piranha::IrCompilationUnit::ParseResult piranha::IrCompilationUnit::parseHelper(
    std::istream &stream, IrCompilationUnit *topLevel) 
{
    delete m_scanner;
    try {
        m_scanner = new piranha::Scanner(&stream);
    }
    catch (std::bad_alloc) {
        return FAIL;
    }

    delete m_parser;
    try {
        m_parser = new piranha::Parser(*m_scanner, *this);
    }
    catch (std::bad_alloc) {
        return FAIL;
    }

    const int success = 0;
    if (m_parser->parse() != success) {
        std::cerr << "Parse failed!!" << std::endl;
        return FAIL;
    }
    else return SUCCESS;
}

piranha::IrNodeDefinition *piranha::IrCompilationUnit::resolveNodeDefinition(
    const std::string &name, int *count, const std::string &libraryName, bool external) 
{
    *count = 0;
    piranha::IrNodeDefinition *firstDefinition = nullptr;
    std::string typeName = name;

    // First search local node definitions if a library is not specified
    if (libraryName.empty()) {
        int localCount = 0;
        IrNodeDefinition *localDefinition = 
            resolveLocalNodeDefinition(typeName, &localCount, external);
        (*count) += localCount;

        if (localDefinition != nullptr) return localDefinition;
    }

    // Search dependencies
    int dependencyCount = getImportStatementCount();
    for (int i = 0; i < dependencyCount; i++) {
        int secondaryCount = 0;
        IrImportStatement *importStatement = getImportStatement(i);

        // Skip the import statement if it already failed
        if (importStatement->getUnit() == nullptr) continue;

        bool libraryNameMatches = importStatement->hasShortName()
            ? libraryName == importStatement->getShortName()
            : false;

        if (libraryName.empty() || libraryNameMatches) {
            // Skip the import statement if it can't be accessed
            if (external && !importStatement->allowsExternalAccess()) continue;

            // The external access flag must be set to true since the libraries are being accessed
            // externally                                                                        ----
            IrNodeDefinition *definition =                                                       ////
                importStatement->getUnit()->resolveNodeDefinition(typeName, &secondaryCount, "", true);
            if (definition != nullptr) {
                (*count) += secondaryCount;

                // Make sure to not overwrite the result definition
                // The first definition to be found must be returned
                if (firstDefinition == nullptr) firstDefinition = definition;
            }
        }
    }

    return firstDefinition;
}

piranha::IrNodeDefinition *piranha::IrCompilationUnit::resolveLocalBuiltinNodeDefinition(
    const std::string &builtinName, int *count, bool external) 
{
    *count = 0;
    piranha::IrNodeDefinition *firstDefinition = nullptr;
    std::string typeName = builtinName;

    int localNodeDefinitions = (int)m_nodeDefinitions.size();
    for (int i = 0; i < localNodeDefinitions; i++) {
        IrNodeDefinition *definition = m_nodeDefinitions[i];
        if (external && !definition->allowsExternalAccess()) continue;
        if (!definition->isBuiltin()) continue;

        if (definition->getBuiltinName() == typeName) {
            (*count)++;
            if (firstDefinition == nullptr) firstDefinition = definition;
        }
    }

    return firstDefinition;
}

piranha::IrNodeDefinition *piranha::IrCompilationUnit::resolveBuiltinNodeDefinition(
    const std::string &builtinName, int *count, bool external) 
{
    piranha::IrNodeDefinition *firstDefinition = nullptr;
    std::string typeName = builtinName;

    // First search local node definitions if a library is not specified
    int localCount = 0;
    IrNodeDefinition *localDefinition = 
        resolveLocalBuiltinNodeDefinition(typeName, &localCount, external);
    (*count) = localCount;

    if (localDefinition != nullptr) return localDefinition;

    // Search dependencies
    int dependencyCount = getImportStatementCount();
    for (int i = 0; i < dependencyCount; i++) {
        int secondaryCount = 0;
        IrImportStatement *importStatement = getImportStatement(i);

        // Skip the import statement if it already failed
        if (importStatement->getUnit() == nullptr) continue;

        // Skip if the import statement cannot be accessed
        if (external && !importStatement->allowsExternalAccess()) continue;

        // The external access flag must be set to true since the libraries are being accessed
        // externally                                                                           ----
        IrNodeDefinition *definition =                                                          ////
            importStatement->getUnit()->resolveBuiltinNodeDefinition(typeName, &secondaryCount, true);
        if (definition != nullptr) {
            (*count) += secondaryCount;
            if (firstDefinition == nullptr) firstDefinition = definition;
        }
    }

    return firstDefinition;
}

piranha::IrNodeDefinition *piranha::IrCompilationUnit::resolveLocalNodeDefinition(
    const std::string &name, int *count, bool external) 
{
    *count = 0;
    piranha::IrNodeDefinition *definition = nullptr;
    std::string typeName = name;

    int localNodeDefinitions = (int)m_nodeDefinitions.size();
    for (int i = 0; i < localNodeDefinitions; i++) {
        IrNodeDefinition *def = m_nodeDefinitions[i];
        if (external && !def->allowsExternalAccess()) continue;

        std::string defName = m_nodeDefinitions[i]->getName();
        if (defName == typeName) {
            (*count)++;
            if (definition == nullptr) definition = m_nodeDefinitions[i];
        }
    }

    return definition;
}

void piranha::IrCompilationUnit::_validate() {
    int nodeCount = getNodeCount();
    for (int i = 0; i < nodeCount; i++) {
        IrNode *node = m_nodes[i];
        int count = countSymbolIncidence(node->getName());

        if (count > 1) {
            this->addCompilationError(new CompilationError(node->getNameToken(),
                ErrorCode::SymbolUsedMultipleTimes));
        }
    }

    int definitionCount = getNodeDefinitionCount();
    for (int i = 0; i < definitionCount; i++) {
        IrNodeDefinition *def = m_nodeDefinitions[i];
        int count = 0;
        resolveLocalNodeDefinition(def->getName(), &count);

        if (count > 1) {
            this->addCompilationError(new CompilationError(*def->getNameToken(),
                ErrorCode::DuplicateNodeDefinition));
        }
    }
}

void piranha::IrCompilationUnit::addNode(IrNode *node) {
    if (node != nullptr) {
        m_nodes.push_back(node);
        registerComponent(node);

        node->setParentUnit(this);
    }
}

piranha::IrNode *piranha::IrCompilationUnit::getNode(int index) const {
    return m_nodes[index];
}

int piranha::IrCompilationUnit::getNodeCount() const {
    return (int)m_nodes.size();
}

void piranha::IrCompilationUnit::addImportStatement(IrImportStatement *statement) {
    if (statement != nullptr) {
        m_importStatements.push_back(statement);
        registerComponent(statement);

        statement->setParentUnit(this);
    }
}

piranha::IrImportStatement * piranha::IrCompilationUnit::getImportStatement(int index) const {
    return m_importStatements[index];
}

int piranha::IrCompilationUnit::getImportStatementCount() const {
    return (int)m_importStatements.size();
}

void piranha::IrCompilationUnit::addNodeDefinition(IrNodeDefinition *nodeDefinition) {
    if (nodeDefinition != nullptr) {
        m_nodeDefinitions.push_back(nodeDefinition);
        registerComponent(nodeDefinition);

        nodeDefinition->setParentUnit(this);
    }
}

piranha::IrNodeDefinition *piranha::IrCompilationUnit::getNodeDefinition(int index) const {
    return m_nodeDefinitions[index];
}

int piranha::IrCompilationUnit::getNodeDefinitionCount() const {
    return (int)m_nodeDefinitions.size();
}

piranha::IrParserStructure *piranha::IrCompilationUnit::resolveLocalName(
    const std::string &name) const 
{
    int nodeCount = getNodeCount();
    for (int i = 0; i < nodeCount; i++) {
        IrNode *node = m_nodes[i];
        if (node->getName() == name) {
            return node;
        }
    }

    return nullptr;
}

int piranha::IrCompilationUnit::countSymbolIncidence(const std::string &name) const {
    int count = 0;
    int nodeCount = getNodeCount();
    for (int i = 0; i < nodeCount; i++) {
        IrNode *node = m_nodes[i];
        if (!name.empty() && node->getName() == name) {
            count++;
        }
    }

    return count;
}

void piranha::IrCompilationUnit::addCompilationError(CompilationError *err) {
    if (m_errorList != nullptr) {
        err->setCompilationUnit(this);
        m_errorList->addCompilationError(err);
    }
    else delete err;
}

std::ostream& piranha::IrCompilationUnit::print(std::ostream &stream) {
    return stream;
}

void piranha::IrCompilationUnit::addDependency(IrCompilationUnit *unit) { 
    m_dependencies.push_back(unit); 
    registerComponent(unit); 
}
