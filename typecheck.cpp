#include "typecheck.hpp"
#include "math.h"

// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable: // In progress
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method: // In progress
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class: // DONE
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member: // In progress
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object: // In progress
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch: // Done? Uncomment various expressions
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch: // TODO
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch: // In progress
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch: // DONE, but uncomment
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case do_while_predicate_type_mismatch: // DONE, but uncomment
      std::cerr << "Predicate of do while loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch: // DONE, but uncomment
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch: // In progress
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch: // DONE
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type: // DONE
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class: // DONE
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present: // DONE
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method: // DONE
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature: // I think this is done...
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

void TypeCheck::visitProgramNode(ProgramNode* node) {
  // Create new classTable, visit children
  classTable = new ClassTable;
  node->visit_children(this);
  
  // Case where no "Main" class exists
  if (classTable->find("Main") == classTable->end()) {
    typeError(no_main_class);
  }
  
  // Case where "Main" class exists
  else {
    // Case where "Main" has members
    if (classTable->find("Main")->second.members->size() > 0) {
        typeError(main_class_members_present);
    }
    // Case where "Main" has no main method
    else if (classTable->find("Main")->second.methods->find("main") == classTable->find("Main")->second.methods->end()) {
      typeError(no_main_method);
    }
    // Case where main method has incorrect signature
    else if (classTable->find("Main")->second.methods->find("main")->second.returnType.baseType != bt_none || classTable->find("Main")->second.methods->find("main")->second.parameters->size() > 0) {
      typeError(main_method_incorrect_signature);
    }
  }

}

void TypeCheck::visitClassNode(ClassNode* node) {
  // Create classInfo + get className
  ClassInfo newClass;
  currentClassName = node->identifier_1->name;

  // Check for superclass
  if (node->identifier_2) {
    newClass.superClassName = node->identifier_2->name;
    
    // Check if superClass already defined
    if (classTable->find(newClass.superClassName) == classTable->end()) {
      typeError(undefined_class);
    }
  }
  else {
    newClass.superClassName = "";
  }

  // Create/update Method/VariableTable
  newClass.methods = new MethodTable;
  newClass.members = new VariableTable;
  currentMethodTable = newClass.methods;
  currentVariableTable = newClass.members;
  // Initalize offsets
  currentMemberOffset = 0;
  currentParameterOffset = 0;

  
  classTable->insert({currentClassName, newClass});

  //Set this to determine if we're visiting member or local vars in visitDeclarationNode
  for (std::list<DeclarationNode*>::iterator it = node->declaration_list->begin(); it != node->declaration_list->end(); ++it) {
    visitDeclarationNode(*it);
  }
  
  // Insert result into classTable
  newClass.membersSize = currentMemberOffset;
  currentParameterOffset = 12;

  // Visit class methods and members (Declarations)
  for (std::list<MethodNode*>::iterator it = node->method_list->begin(); it != node->method_list->end(); ++it) {
    visitMethodNode(*it);
  }

}

void TypeCheck::visitMethodNode(MethodNode* node) {
  // Initalize offset
  currentLocalOffset = 0;

  // Make new methodInfo object
  MethodInfo newMethod;

  // Create compound type for method
  CompoundType methodType;

  newMethod.returnType = methodType;

  // Set variableTable, parameterList, and currentLocalOffest
  newMethod.variables  = new VariableTable;
  newMethod.parameters = new std::list<CompoundType>();

  currentVariableTable = newMethod.variables;

  //Determines return type of the method idk why it works but it do
  node->visit_children(this);
  newMethod.returnType.baseType = node->type->basetype;
 
  if (newMethod.returnType.baseType == bt_object) {
    newMethod.returnType.objectClassName = node->type->objectClassName;
    
    // Check if objectClassName already defined
    if (classTable->find(newMethod.returnType.objectClassName) == classTable->end()) {
      typeError(undefined_class);
    }
  }
  else {
    newMethod.returnType.objectClassName = node->identifier->name;
  }

  // Set MethodNode basetype; check if basetype is object
  node->basetype = node->type->basetype;
  if (node->basetype == bt_object) {
    node->objectClassName = node->type->objectClassName;
  }

  // Check if the method is an invalid class constructor 
  if (node->identifier->name == currentClassName && node->type->basetype != bt_none) {
    typeError(constructor_returns_type);
  }

  //Local offset of 12 + each param is 4
  currentParameterOffset = 12;

  for (std::list<ParameterNode*>::iterator it = node->parameter_list->begin(); it != node->parameter_list->end(); ++it) {
    //visitParameterNode(*it);
    // Create compoundtype for Parameters
    CompoundType newParam;
    newParam.baseType = (*it)->type->basetype;
    newParam.objectClassName = (*it)->type->objectClassName;
    //std::cout << "Parameter : " << (*it)->identifier->name << " type is " << string(newParam) << "\n\n";
    newMethod.parameters->push_back(newParam);
  }

  //Reset currentParam node after visiting all params
  currentParameterOffset = 12;

  // Check if return type doesn't match
  
  // Check if return type is none
  if (node->type->basetype == bt_none) {
    // If there's a return here, the types don't match
    if (node->methodbody->returnstatement) {
      //std::cout << "Basetype is none, function has return node\n\n";
      typeError(return_type_mismatch);
    }
  }
  // Else, the function must return something
  else {
    // If there's no return OR the types don't match...
    if (!(node->methodbody->returnstatement) || node->methodbody->returnstatement->basetype != node->type->basetype) {
      typeError(return_type_mismatch);
    }
  }

  // Set localsSize, insert into current methodTable
  newMethod.localsSize = abs(currentLocalOffset);
  currentMethodTable->insert({node->identifier->name, newMethod});

}

void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
  // if (debug)
  //   std::cout << "Visiting methodBody node\n\n";
  // Visit Declaration list, statement list, and return statment
  node->visit_children(this);

  // If methodBody has a return... (necessary to have this?)
  if (node->returnstatement) {
     node->basetype = node->returnstatement->basetype;
    // Check if basetype is an object
    if (node->returnstatement->basetype == bt_object) {
      node->objectClassName = node->returnstatement->objectClassName;
    }
  }

}

void TypeCheck::visitParameterNode(ParameterNode* node) {
  // if (debug)
  //   std::cout << "Visiting parameter node\n\n";
  node->visit_children(this);
  // Make new variableInfo object
  VariableInfo newVariable;

  // Create compound type for variable
  CompoundType variableType;

  // Set fields, assign to newVariable
  variableType.baseType = node->type->basetype;
  if (variableType.baseType == bt_object) {
    variableType.objectClassName = node->type->objectClassName;
    
    // Check if objectClassName is in classTable
    if (classTable->find(variableType.objectClassName) == classTable->end()) {
      typeError(undefined_class);
    }
  }
  else {
    variableType.objectClassName = node->identifier->name;
  }

  newVariable.type = variableType;

  newVariable.size = 4;
  newVariable.offset = currentParameterOffset;
  currentParameterOffset += 4;

  currentVariableTable->insert({node->identifier->name, newVariable});
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) {
  // if (debug)
  //   std::cout << "Visiting declaration node\n\n";
  node->visit_children(this);

  for (auto it = node->identifier_list->begin(); it != node->identifier_list->end(); ++it){
    //Make new variable type per declaration ex: int a,b,c,d ... 
    CompoundType variableType;

    //Variable belongs to object with currentClassName
    variableType.baseType = node->type->basetype; 

    // Check if basetype is an object
    if (variableType.baseType == bt_object) {
       variableType.objectClassName = node->type->objectClassName;
       
       // Check if objectClassName is in classTable
       if (classTable->find(variableType.objectClassName) == classTable->end()) {
        typeError(undefined_class);
      }
    }

    //Make new varInfo and set it's type
    VariableInfo newVariable;
    newVariable.type = variableType;

    //Set size and update offset
    newVariable.size    = 4;
    currentLocalOffset -= 4;

    //Set offset and determine if var is heap/stack allocated

    if (currentParameterOffset == 0)
      newVariable.offset = currentMemberOffset;
    else
      newVariable.offset = currentLocalOffset;

    //Insert to variable table
    currentVariableTable->insert({(*it)->name, newVariable});
    currentMemberOffset += 4;
  }
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
  // if (debug)
  //   std::cout << "Visiting return statement node\n\n";
	//Evaluate expression for basetype
    node->visit_children(this);
    node->basetype = node->expression->basetype;

    // Check if basetype is an object
    if (node->expression->basetype == bt_object) {
      node->objectClassName = node->expression->objectClassName;
  } 

}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
  // if (debug)
  //   std::cout << "Visiting assignment node\n\n";
  node->visit_children(this);

  bool foundID1 = false;

  // Determine ID1 baseType
  std::string ID1Name = node->identifier_1->name;
  CompoundType ID1, ID2;

  if (currentVariableTable->find(ID1Name) != currentVariableTable->end()) {
    foundID1 = true;
    ID1.baseType = currentVariableTable->find(ID1Name)->second.type.baseType;
    if (ID1.baseType == bt_object) {
      ID1.objectClassName = currentVariableTable->find(ID1Name)->second.type.objectClassName;
    }
  }

  else {

    if (classTable->find(currentClassName)->second.members->find(ID1Name) != classTable->find(currentClassName)->second.members->end()) {
      foundID1 = true;
      ID1.baseType = classTable->find(currentClassName)->second.members->find(ID1Name)->second.type.baseType;
      if (ID1.baseType == bt_object) {
        ID1.objectClassName = classTable->find(currentClassName)->second.members->find(ID1Name)->second.type.objectClassName;
      }
    }

    else {

      std::string superClass = classTable->find(currentClassName)->second.superClassName;

      while (superClass != "") {

        if (classTable->find(superClass)->second.members->find(ID1Name) != classTable->find(superClass)->second.members->end()) {
          foundID1 = true;
          ID1.baseType = classTable->find(superClass)->second.members->find(ID1Name)->second.type.baseType;
          if (ID1.baseType == bt_object) {
            ID1.objectClassName = classTable->find(superClass)->second.members->find(ID1Name)->second.type.objectClassName;
          }
          break;
        }
        else {
          superClass = classTable->find(superClass)->second.superClassName;
        }
      }
    }
  }

  if (!foundID1) {
    typeError(undefined_variable);
  }

 
  // Check if Expr is of form (class.member)
  if (node->identifier_2) {

    // Check if identifier_1 is an object
    if (ID1.baseType != bt_object) {
      typeError(not_object);
    }
    
    // Check if class does in fact declare the member
    if (classTable->find(ID1.objectClassName) == classTable->end()) {
      typeError(undefined_class);
    }

    ClassInfo currClass = classTable->find(ID1.objectClassName)->second;

    // If we CAN find the member in the current class
    if (currClass.members->find(node->identifier_2->name) != currClass.members->end()){
      node->basetype = currClass.members->find(node->identifier_2->name)->second.type.baseType;
      ID2.baseType = node->basetype;
      if (node->basetype == bt_object) {
        node->objectClassName = currClass.members->find(node->identifier_2->name)->second.type.objectClassName;
        ID2.objectClassName = node->objectClassName;
      }
    }

    // If we can't find the member in the current class...
    else {
      // If the current class doesn't have a superclass...
      if (currClass.superClassName == "") {
        // if (debug) 
        //   std::cout << "Assignment Node: current class has no superclass\n\n";
        typeError(undefined_member);
      }

      // Else, there must be a super class...
      else {
        std::string superClass = currClass.superClassName;
        int found = 0;
        // While we keep getting superclasses...
        while (superClass != "") {
          // If we can't find ID2 in the superClass' members...
          if (classTable->find(superClass)->second.members->find(node->identifier_2->name) == classTable->find(superClass)->second.members->end()) {
            superClass = classTable->find(superClass)->second.superClassName;
          }
          // Else we've found it!
          else {
            found = 1;
            break;
          }
        }
        // If we found the member...
        if (found) {
          // if (debug) 
          //   std::cout << "Found member in superclass: " + superClass + "\n\n";
          node->basetype = classTable->find(superClass)->second.members->find(node->identifier_2->name)->second.type.baseType;
          ID2.baseType = node->basetype;
          if (node->basetype == bt_object) {
            node->objectClassName = classTable->find(superClass)->second.members->find(node->identifier_2->name)->second.type.objectClassName;
            ID2.objectClassName = node->objectClassName;
          }
        }
        // Unable to find member
        else {
          // if (debug)
          //   std::cout << "Assignment Node: unable to find class member\n\n";
          typeError(undefined_member);
        }
      }
    }
   
    // Check if basetypes are the same
    if (node->identifier_2) {
      if (ID2.baseType != node->expression->basetype) {
        // CompoundType expr;
        // expr.baseType = node->expression->basetype;
        //std::cout << "Case where ID2 exists\n\n";
        //std::cout << "Assignment node: " + string(ID2) + " vs. " + string(expr) + "\n\n"; 
        typeError(assignment_type_mismatch);
      }
    }
     
    else {
      if (ID1.baseType != node->expression->basetype) {
        // CompoundType expr;
        // expr.baseType = node->expression->basetype;
        //std::cout << "Case where ID2 DOES NOT exist\n\n";  
        //std::cout << "Assignment node: " + string(ID1) + " vs. " + string(expr) + "\n\n"; 
    
        typeError(assignment_type_mismatch);
      }
    }
  }
 
  // Expr of form (identifier = expr)
  else {
  // Check if basetypes are the same
    if (ID1.baseType != node->expression->basetype) {
    // CompoundType expr;
    // expr.baseType = node->expression->basetype;
    // if (debug) {
    //   std::cout << "Assignment node: " + string(ID1) + " vs. " + string(expr) + "\n\n"; 
    //   std::cout << "ID1Name: " << ID1Name << "\n\n";
    // }

    typeError(assignment_type_mismatch);
    }
  }
  
  node->basetype = node->expression->basetype;
  if (node->basetype == bt_object) {
    node->objectClassName = node->expression->objectClassName;
  }

}

void TypeCheck::visitCallNode(CallNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);


}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
  // if (debug)
  //   std::cout << "Visiting IfElse node\n\n";
  //   Evaluate expression return type
  node->visit_children(this);

	// Check that return type for condition is a boolean
	if (node->expression->basetype != bt_boolean){
		typeError(if_predicate_type_mismatch);
	}
}

void TypeCheck::visitWhileNode(WhileNode* node) {
  // Evaluate expression return type
  // if (debug)
  //   std::cout << "Visiting While node\n\n";
  node->visit_children(this);

	// Check that return type for condition is a boolean
	if (node->expression->basetype != bt_boolean){
		typeError(while_predicate_type_mismatch);
	}
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
  // Evaluate expression type
	node->visit_children(this);

	if (node->expression->basetype != bt_boolean){
		typeError(do_while_predicate_type_mismatch);
	}
}

void TypeCheck::visitPrintNode(PrintNode* node) {
  // Just expand the expression here
  //std::cout << "Visiting Print Node\n\n";
  node->visit_children(this);
  node->expression->visit_children(this);
}

void TypeCheck::visitPlusNode(PlusNode* node) {
  // if (debug)
  //   std::cout << "Visiting plus node\n\n";
  node->visit_children(this);
  
  // Check if basetypes match && are ints
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_integer;
}

void TypeCheck::visitMinusNode(MinusNode* node) {
  // if (debug)
  //   std::cout << "Visiting minus node\n\n";
  node->visit_children(this);
  
  // Check if basetypes match && are ints
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_integer;
}

void TypeCheck::visitTimesNode(TimesNode* node) {
  // if (debug)
  //   std::cout << "Visiting times node\n\n";
  node->visit_children(this);

  // Check if basetypes match && are ints
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_integer;
}

void TypeCheck::visitDivideNode(DivideNode* node) {
  node->visit_children(this);
  
  // Check if basetypes match && are ints
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_integer;
}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
  // if (debug)
  //   std::cout << "Visiting greater node\n\n";
  node->visit_children(this);
  
  // Check if basetypes match && are ints
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_boolean;
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
  // if (debug)
  //   std::cout << "Visiting greater or equal node\n\n";
  node->visit_children(this);
  
  // Check if basetypes match && are ints
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_boolean;
}

void TypeCheck::visitEqualNode(EqualNode* node) {
  // if (debug)
  //   std::cout << "Visiting equal node\n\n";
  node->visit_children(this);
  
  // Check if expressions are booleans OR ints
  if ((node->expression_1->basetype == bt_integer && node->expression_2->basetype == bt_integer) || (node->expression_1->basetype == bt_boolean && node->expression_2->basetype == bt_boolean)) {
    node->basetype = bt_boolean;
    return;
  }
  else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitAndNode(AndNode* node) {
  // if (debug)
  //   std::cout << "Visiting and node\n\n";
  node->visit_children(this);
  
  // Check if basetypes match && are booleans
  if (node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_boolean;
}

void TypeCheck::visitOrNode(OrNode* node) {
  // if (debug)
  //   std::cout << "Visiting or node\n\n";
  node->visit_children(this);
  
  // Check if basetypes match && are booleans
  if (node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_boolean;
}

void TypeCheck::visitNotNode(NotNode* node) {
  // if (debug)
  //   std::cout << "Visiting not node\n\n";
  node->visit_children(this);
  
  // Check if basetype is boolean
  if (node->expression->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_boolean;
}

void TypeCheck::visitNegationNode(NegationNode* node) {
  // if (debug)
  //   std::cout << "Visiting negation node\n\n";
  node->visit_children(this);
  
  // Check if expression is bt_int
  if (node->expression->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  
  node->basetype = bt_integer;
}

void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
  // if (debug)
  //   std::cout << "Visiting MethodCallNode " << std::endl;
	node->visit_children(this);

	//Init some values to be used throughout checking process
	std::string methodName;
	std::string callingClassName;
  std::string classContainingMethod;
  std::string objectCName = "";

	bool methodFound;
	bool classFound;


  //Case: class.method(arg1, arg2, ... )
	//TODO: Check if class is inherrited from super class and check if method is inherrited from a super class 
  if (node->identifier_2){

    //Name of the variable
		callingClassName = node->identifier_1->name;
		methodName  = node->identifier_2->name;

    // if (debug)
    //   std::cout << "class method call of form : " << callingClassName + "." + methodName << std::endl;

    // Determining ID type through current table
    CompoundType ID1;

    //Check local vars
    if (currentVariableTable->find(callingClassName) != currentVariableTable->end()) {
      ID1.baseType = currentVariableTable->find(callingClassName)->second.type.baseType;
      objectCName  = currentVariableTable->find(callingClassName)->second.type.objectClassName;
      classFound = true;
    }

    //Check current members
    VariableTable *currentMemberTable = (*classTable)[currentClassName].members;

    if (currentMemberTable->count(callingClassName) != 0){
      ID1.baseType = currentMemberTable->find(callingClassName)->second.type.baseType;
      objectCName  = currentMemberTable->find(callingClassName)->second.type.objectClassName;
      classFound   = true;
    }


    //We didn't find it, need to start searching superClasses
    if (currentVariableTable->count(callingClassName) == 0){
      std::string superClass = (*classTable)[currentClassName].superClassName;

      while (superClass != ""){

        VariableTable *vi = (*classTable)[superClass].members;

        if (vi->count(callingClassName) != 0){
          ID1.baseType = vi->find(callingClassName)->second.type.baseType;
          objectCName  = vi->find(callingClassName)->second.type.objectClassName;
          classFound = true;
          break; 
        }

        superClass = (*classTable)[superClass].superClassName;

        
      }

    }

    if (!classFound) {
      typeError(undefined_variable);
    }
    //Check that callingClassName is an actual variable of type class
    if (ID1.baseType != bt_object)
      typeError(not_object);

		//Might need to check if method is inherrited 
    MethodTable *mTable = (*classTable)[objectCName].methods;

    //Check current object methods
    if (mTable->count(methodName) != 0){
      methodFound = true;
    }

    //Check if inherrited
    if (!methodFound){
      std::string superClassName = (*classTable)[objectCName].superClassName;

      while(superClassName != ""){

        //std::cout << "Now looking for method " + methodName + " in class " + superClassName << std::endl;
        
        if ( (*classTable)[superClassName].methods->count(methodName) != 0 ){
          methodFound = true;
          objectCName = superClassName;
          break;
        }

        superClassName = (*classTable)[superClassName].superClassName;
      }

    }

    if (!methodFound){
      typeError(undefined_method);
    }
		
  }

  //Case method(arg1, arg2, ...)
	if (!node->identifier_2){

		//Check if defined in localVars or memberVars
		methodName = node->identifier_1->name;
    //std::cout << "Lone method call for method of name : " << methodName << std::endl;

    // if (debug)
    //   std::cout << "methodName is :" << methodName << std::endl;

		//Method table to check
		std::map<std::string, MethodInfo>* methods = currentMethodTable;

		//We found our method in our current methodTable
		if (methods->count(methodName) != 0){
      methodFound = true;
      objectCName = currentClassName;
		}

		//If we didn't find the method, we need to see if it's inherrited from a superclass
		else {
      std::string superClassName = (*classTable)[currentClassName].superClassName;

      //For each super class, check if the method is defined
      while (superClassName != ""){
        MethodTable* superClassMethods = (*classTable)[superClassName].methods;

        //We found the className and everthing is fine
        if (superClassMethods->count(methodName) != 0){
          //std::cout << "found method : " + methodName + " in class " + superClassName << std::endl;
          methodFound = true;
          objectCName = superClassName;

          break;
        }

        //Else we didn't find the method so we update the superclass and keep looking
        superClassName = (*classTable)[superClassName].superClassName;
      }
    }

		//After checking all the superclasses we didn't find the method so throw an error
		if (!methodFound){
			typeError(undefined_method);
		}

	}

	//Now we need to check that the parameters and arguments match
	BaseType argType, paramType;


  //std::cout << "Attempting to find method info for method " + methodName + " in class " + objectCName << std::endl;


  //This line segfaults --> we don't find the method
  if ( (*classTable)[objectCName].methods->find(methodName) == (*classTable)[objectCName].methods->end() )
    ;
    //std::cout << "WE DIDN'T FIND THE METHOD BUT WE SHOULD HAVE THROWN AN ERROR EARLIER OOPSIE WOOPSIE" << std::endl;

  MethodInfo mi = (*classTable)[objectCName].methods->find(methodName)->second;

  //Set the return type
  node->basetype = mi.returnType.baseType;

	std::list<ExpressionNode*>::iterator args = node->expression_list->begin();
	std::list<CompoundType>::iterator  params = mi.parameters->begin();

	//Iterate over args and parameters, ensuring each type matches
	while (args != node->expression_list->end() && params != mi.parameters->end() ){
		argType   = (*args)->basetype;
		paramType = params->baseType;

		if (argType != paramType){
			typeError(argument_type_mismatch);			
		}

		++args;
		++params;
	}

	//If both iterators do not equal end, then we have an unequal number of params
	if (args != node->expression_list->end() || params != mi.parameters->end()){
		typeError(argument_number_mismatch);
	}

}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
  // // WRITEME: Replace with code if necessary
  node->visit_children(this);

  // Determine ID1 baseType
  std::string ID1Name = node->identifier_1->name;
  std::string ID2Name = node->identifier_2->name;
  CompoundType ID2;

  bool foundMember = false;

  if (node->identifier_1->basetype != bt_object){
    // if (debug)
    //   std::cout << ID1Name << " is not an object" << std::endl;
    typeError(not_object);
  }

  //Check current class
  if (currentVariableTable->find(ID2Name) != currentVariableTable->end()) {
    ID2.baseType = currentVariableTable->find(ID2Name)->second.type.baseType;
    if (ID2.baseType == bt_object) {
      ID2.objectClassName = currentVariableTable->find(ID2Name)->second.type.objectClassName;
    }
    foundMember = true;
  }

  //Check super classes
  else {
    std::string superClassName = classTable->find(currentClassName)->second.superClassName;

    while (superClassName != ""){

      //Look in the members of the super class for our thing
      VariableTable *members = (*classTable)[superClassName].members;

      if (members->count(ID2Name) != 0){
        ID2.baseType = members->find(ID2Name)->second.type.baseType;
        foundMember = true;

        if (ID2.baseType == bt_object) {
          ID2.objectClassName = members->find(ID2Name)->second.type.baseType;
        }
        break;
      }
      // superClassName = (*classTable)[superClassName].superClassName;
      else {
        superClassName = classTable->find(superClassName)->second.superClassName;
      }
    }

    // Check if objectClassName of ID1 contains member
    std::string objectClassName1;
    if (currentVariableTable->find(ID1Name) != currentVariableTable->end()) {
      objectClassName1 = currentVariableTable->find(ID1Name)->second.type.objectClassName;
    }

    else if (classTable->find(currentClassName)->second.members->find(ID1Name) != classTable->find(currentClassName)->second.members->end()) {
      objectClassName1 = classTable->find(currentClassName)->second.members->find(ID1Name)->second.type.objectClassName;
    }

    else {
      std::string superClass = classTable->find(currentClassName)->second.superClassName;
      while (superClass != "") {
        if (classTable->find(superClass)->second.members->find(ID1Name) != classTable->find(superClass)->second.members->end()) {
          objectClassName1 = classTable->find(superClass)->second.members->find(ID1Name)->second.type.objectClassName;
          break;
        }
        else {
          superClass = classTable->find(superClass)->second.superClassName;
        }
      }
    } 
    
    while (objectClassName1 != "") {
      if (classTable->find(objectClassName1) != classTable->end()) {
        if (classTable->find(objectClassName1)->second.members->find(ID2Name) != classTable->find(objectClassName1)->second.members->end()) {
          ID2.baseType = classTable->find(objectClassName1)->second.members->find(ID2Name)->second.type.baseType;
          if (ID2.baseType == bt_object) {
            ID2.objectClassName = classTable->find(objectClassName1)->second.members->find(ID2Name)->second.type.baseType;
          }
          foundMember = true;
          break;
        }
        else {
          objectClassName1 = classTable->find(objectClassName1)->second.superClassName;
        }
      }
    }
  }

  if (!foundMember) {
    // if (debug) {
    //   std::cout << "Looking for member: " + ID2Name + "\n\n";
    //   std::cout << "Object: " << ID1Name << "\n\n";
    //   std::cout << "MemberAccess Node: member not found\n\n";
    // }
    typeError(undefined_member);
  }

  node->basetype = ID2.baseType;
  if (node->basetype == bt_object) {
    node->objectClassName = ID2.objectClassName;
  }

}

void TypeCheck::visitVariableNode(VariableNode* node) {
  // if (debug)
  //   std::cout << "Visiting variable node\n\n";
  node->visit_children(this);

  std::string varName = node->identifier->name;
  //std::cout << "varName is: " << varName << "\n\n";
  // Check if variable is undefined
  
  // If variable in current variable table
  if (currentVariableTable->find(varName) != currentVariableTable->end()) {
    node->basetype = currentVariableTable->find(node->identifier->name)->second.type.baseType;
     // Check if variable is an object
    if (node->basetype == bt_object) {
      node->objectClassName = currentVariableTable->find(varName)->second.type.objectClassName;
    }
  } 

  // Else, if variable is a class member
  else if (classTable->find(currentClassName)->second.members->find(varName) != classTable->find(currentClassName)->second.members->end()) {
    node->basetype = classTable->find(currentClassName)->second.members->find(varName)->second.type.baseType;

    // Check if variable is an object
    if (node->basetype == bt_object) {
      node->objectClassName = classTable->find(currentClassName)->second.members->find(varName)->second.type.objectClassName;
    }
  }

  // Else, variable may be in parameters


  // Else, variable may be in superclass(es)
  else if (classTable->find(currentClassName)->second.members->find(varName) == classTable->find(currentClassName)->second.members->end()) {
    std::string superClass = classTable->find(currentClassName)->second.superClassName;
    bool inSuperClass = false;

    while (superClass != "") {
      if (classTable->find(superClass)->second.members->find(varName) != classTable->find(superClass)->second.members->end()) {
        inSuperClass = true;
        break;
      }
      superClass = (*classTable)[superClass].superClassName;
    }

    if (inSuperClass) {
      node->basetype = classTable->find(superClass)->second.members->find(varName)->second.type.baseType;
      // Check if basetype is object
      if (node->basetype == bt_object) {
        node->objectClassName = classTable->find(superClass)->second.members->find(varName)->second.type.objectClassName;
      }
    }

    else {
      typeError(undefined_variable);
    }
  }
  
  else {

    typeError(undefined_variable);
  }

  CompoundType var;
  var.baseType = node->basetype;
  //std::cout << "Variable node basetype is: " << string(var) << "\n\n";
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
  // if (debug)
  //   std::cout << "Visiting int literal node\n\n";
  node->basetype = bt_integer;
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
  // if (debug)
  //   std::cout << "Visiting boolean literal node\n\n";
  node->basetype = bt_boolean;
}

void TypeCheck::visitNewNode(NewNode* node) {
  node->visit_children(this);

  // See if "new" class exists
  if (classTable->find(node->identifier->name) == classTable->end()) {
    typeError(undefined_class);
  }

  //Check that the constructor expects arguments
  if (node->expression_list){
    std::string objectCName = node->identifier->name;
    MethodTable *constructor = (*classTable)[objectCName].methods;

    //Check constructor exists
    if (constructor->count(objectCName) == 0)
      typeError(undefined_method);


    //Else check the variable types match and have same args
    MethodInfo mi = constructor->find(objectCName)->second;

    std::list<ExpressionNode*>::iterator args = node->expression_list->begin();
    std::list<CompoundType>::iterator  params = mi.parameters->begin();

    BaseType argType, paramType;

    //Iterate over args and parameters, ensuring each type matches
    while (args != node->expression_list->end() && params != mi.parameters->end() ){
      argType   = (*args)->basetype;
      paramType = params->baseType;

      if (argType != paramType){
        typeError(argument_type_mismatch);			
      }

      ++args;
      ++params;
    }

    //If both iterators do not equal end, then we have an unequal number of params
    if (args != node->expression_list->end() || params != mi.parameters->end()){
      typeError(argument_number_mismatch);
    }

  }




  node->basetype = bt_object;
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
  node->basetype = bt_integer;
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
  node->basetype = bt_boolean;
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
}

void TypeCheck::visitNoneNode(NoneNode* node) {
  node->basetype = bt_none;
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {
  // WRITEME: Replace with code if necessary

  node->basetype = bt_object;

  // CompoundType temp;
  // std::string baseT = string(temp);
  // std::cout << "ID BaseType: " + baseT + "\n\n";

}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
  // WRITEME: Replace with code if necessary
}

// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}
