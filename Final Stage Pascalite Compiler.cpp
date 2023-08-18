//Stage 2 Compiler
//Dylan McClellan
//Michael Stillabower
//Ryan Morris
//CS 4301

//we got tons of help from Joel, Womack, AJ, Gavin, Lilly, and Leo

//Notes: always make sure when we need to press forward so always ensure token is behaving properly. If token is screwed, the rest is screwed ***REMEMBER THIS***

#include <ctime>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <ctype.h>
#include <map>
#include <cctype>
//#include <stage1.h>
#include <stage2.h>
#include <iomanip>
#include <stack>
#include <vector>
#include <string>

using namespace std;
char chToHold = '\n';
//prototypes
bool isRelOperator(string);
bool isAddLevelOperator(string);
bool isMultLevelOperator(string);
//bool locatedInSymbolTable(string);

Compiler::Compiler(char **argv)  
{//NO TEST LINES  constructor
    sourceFile.open(argv[1]);
    listingFile.open(argv[2]);
    objectFile.open(argv[3]);
}
Compiler::~Compiler()       
{//NO TEST LINES  destructor
    sourceFile.close();
    listingFile.close();
    objectFile.close();
}
/**First 3 Member Functions**/
void Compiler::createListingHeader()
{
	time_t now = time(NULL);
    listingFile << left << "STAGE1:\t" << "Dylan McClellan, Michael Stillabower, Ryan Morris\t" << ctime(&now) << endl;
    listingFile << left << setw(22) << "LINE NO." << "SOURCE STATEMENT\n\n";
    //line numbers and source statements should be aligned under the headings
}
void Compiler::parser()
{
    //ch must be initialized to the first character of the source file
	nextChar();
	if (nextToken() != "program")
    {
        processError("keyword \"program\" expected");       //error case for program
    }
    prog();
	//parser implements the grammar rules, calling first rule
}
void Compiler::createListingTrailer()
{
	if (errorCount == 1)
		listingFile << endl << "COMPILATION TERMINATED"<< setw(7) << errorCount << " ERROR ENCOUNTERED"<< endl;
	else
		listingFile << endl << "COMPILATION TERMINATED"<< setw(7)<< errorCount << " ERRORS ENCOUNTERED"<<endl;
}
/**END FIRST 3 MEMBER FUNCTIONS**/

/**Grammar Productions**/
void Compiler::prog()                                               
{//token should be "program"
    if (token != "program")
    {
        processError("keyword 'program' expected");
    }
    progStmt();
    if (token == "const")
    {
        consts();
    }
    if (token == "var")
    {
        vars();
    }
    if (token != "begin")
    {
        processError("keyword 'begin' expected");
    }
    beginEndStmt();
    if (token[0] != END_OF_FILE)
    {
        processError("no text may follow end");
    }
}
void Compiler::progStmt()                                           
{//token should be "program"
    string x;
    if (token != "program")
    {
        processError("keyword 'program' expected");
    }
    x = nextToken();
    if (!isNonKeyId(token))
    {
        processError("program name expected");
    }
	nextToken();
    if (token != ";")
    {
        processError("semicolon expected");
    }
    nextToken();
    code("program", x);
	insert(x,PROG_NAME,CONSTANT,x,NO,0);
}
void Compiler::consts()                                             
{//goto constStmts
    if (token != "const")
    {
        processError("keyword \"const\" expected");
    }
    nextToken();
    if (!isNonKeyId(token))
    {
        processError("non-keyword identifier must follow 'const'");
    }
    constStmts();
}
void Compiler::vars()
{
    if (token != "var")
    {
        processError("keyword \"var\" expected");
    }
    if (!isNonKeyId(nextToken()))
    {
        processError("non-keyword identifier must follow \"var\"");
    }
    varStmts();
}
void Compiler::beginEndStmt()
{
	static stack<int> stkStart;
	stkStart.push(1);
   
    if (token != "begin")
    {
        processError("keyword \"begin\" expected");
    }
	
	nextToken();
	
	if (isNonKeyId(token) || token == "read" || token == "write" || token == "if" || token == "while" || token == "repeat" || token == ";" || token == "begin")
	{
		execStmts(); //MODIFIED FOR STAGE 1                                                                                                    
	}
		
	if (token != "end" && !isNonKeyId(token) && token != "read" && token != "write" && token != "if" && token != "while" && token != "repeat" && token != ";")
    {
        processError("keyword \"end\" , NON_KEY_ID, \"read\", or \"write\" expected");
    }
	
	nextToken();
	
    if (token != "." && token != ";")
    {
        processError("period or semicolon expected after \"end\"");
    }
	
	if(stkStart.size() > 1 && token == ".")
	{
		processError("end must conclude with ';' unless is final end, \n possible wrong conclusion symbols or num of begins and ends");
	}
	if(stkStart.size() == 1 && token == ".")
	{
		code("end", ".");
	}
	
	if (token != ";")
    {
		nextToken();
    }
	//code("end", "."); moved for stage2
	stkStart.pop();
}
void Compiler::constStmts()		
{           
    string x,y;
    if (!isNonKeyId(token))
        processError("non-keyword identifier expected");
    x = token;
	nextToken();
    if (token != "=")
    {
        processError("\"=\" expected");
    }
    y = nextToken();
    if (y != "+" && 
		y != "-" && 
		y != "not" && 
		y != "true" && 
		y != "false" && 
		!isNonKeyId(y) && 
		!isInteger(y) && 
		!isBoolean(y))
	{
        processError("token to right of \"=\" illegal");
    }
    if (y == "+" || y == "-")
    {
		nextToken();
        if (!isInteger(token))
        {
            processError("integer expected after sign");
        }
        y = y + token;
    }
    if (y == "not")
    {
		nextToken();
        if (!isBoolean(token) && whichType(token) != BOOLEAN)
        {
            processError("boolean expected after 'not'");
        }
		
        if (token == "true" || whichValue(token) == "true")
		{
			y = "false";
		}			
        else
            y = "true";
    }
    token = nextToken();
    if (token != ";")
    {
        processError("semicolon expected");
    }
    if (whichType(y) != INTEGER && whichType(y) != BOOLEAN)
    {
        processError("data type of token on the right-hand side must be INTEGER or BOOLEAN");
    }
	
	insert(x,whichType(y),CONSTANT,whichValue(y),YES,1);
    
	x = nextToken();
    if (x != "begin" && x != "var" && !isNonKeyId(x))
    {
        processError("non-keyword identifier, 'begin', or 'var' expected");
    }
    if (isNonKeyId(x))
    {
        constStmts();
    }
}
void Compiler::varStmts()		
{
	string x,y;
    if (!isNonKeyId(token))
    {
        processError("non-keyword identifier expected");
    }
    x = ids();
    if (token != ":")
    {
        processError("\":\" expected");
    }
    nextToken();
    if (token != "integer" && token != "boolean")
    {
        processError("illegal type follows \":\"");
    }
    y = token;
	nextToken();
    if (token != ";")
    {
        processError("semicolon expected");
    }
    if (y == "integer")
	{
		
		insert(x,INTEGER,VARIABLE,"",YES,1);
		
	}
	else if(y == "boolean")
	{
		insert(x,BOOLEAN,VARIABLE,"",YES,1);
	}
	nextToken();
    if (token != "begin" && !isNonKeyId(token))
    {
        processError("non-keyword identifier or \"begin\" expected");
    }
    if (isNonKeyId(token))
        varStmts();
}
string Compiler::ids()
{
    string temp,tempString;
		if(!isNonKeyId(token))
		{
			processError("non-keyword identifier expected");
		}
		tempString = token;
		temp = token;
		nextToken();
		if (token == ",")
		{
			nextToken();
			if(!isNonKeyId(token)){
				processError("non-keyword identifier expected");
			}
			tempString = temp + "," + ids();
		}
		if (temp.back() == '_')
		{
			processError("non-keyword identifier cannot end with \"_\"");
		}
		return tempString;
}
/**END GRAMMAR**/

/**STAGE 1 GRAMMAR FUNCTIONS**/
void Compiler::execStmts()
{//goto execStmt 
    
	//stage 1 iteration of execStmts
	//while (isNonKeyId(token)
    //                || token == "read"
    //                || token == "write")
    //{
    //    execStmt();
	//	execStmts();
    //}
	
	while(token != "until" && token != "end")
	{
		execStmt();
		
		//if token is a semicolon we can proceed the token
		if (token == ";")
		{
			nextToken();
		}
	}
    
}
void Compiler::execStmt()
{
	if (isNonKeyId(token))
    {
        assignStmt();
    }
    else if (token == "read")
    {
        readStmt();
    }
    else if (token == "write")
    {
        writeStmt();
    }
    else if (token == "if")                //Stage 2 revised
    {
        ifStmt();
    }
    else if (token == "while")             //Stage 2 revised
    {
        whileStmt();
    }
    else if (token == "repeat")            //Stage 2 revised
    {
        repeatStmt();
    }
    else if (token == ";")                 //Stage 2 revised
    {
        nullStmt();
    }
    else if (token == "begin")             //Stage 2 revised
    {
        beginEndStmt();
    }
	else
		processError("one of \";\", \"begin\", \"if\", \"read\", \"repeat\", \"while\", \"write\", \"end\", or \"until\" expected");
}
void Compiler::assignStmt()
{//input -> NON_KEY_ID		output -> NON_KEY_ID ':=' EXPRESS ';'
	if (!isNonKeyId(token))
    {
        processError("non-keyword identifier expected");
    }
	
    pushOperand(token);
    nextToken();
	
    if (token == ":=")
    {
		pushOperator(token);
		nextToken();
		express();
		
		if (token != ";")
		{
			processError("one of \"*\", \"and\", \"div\", \"mod\", \")\", \"+\", \"-\", \";\", \"<\", \"<=\", \"<>\", \"=\", \">\", \">=\", or \"or\" expected");
		}
	}
	else
	{
		processError("':=' expected");
	}
 
    string rightHandBound = popOperand();
    string leftHandBound = popOperand();
    code(popOperator(), rightHandBound, leftHandBound);
}
void Compiler::readStmt()
{//input -> 'read'  output -> 'read' READ_LIST ';'		READ_LIST -> '(' IDSx ')' code('read',x)
	string listOfIds, temp;
	temp = "temp";
	
	if(token != "read")
	{
		processError("Keyword \"read\" expected instead of: " + token);
	}
    nextToken();
	
    if (token != "(")
    {
        processError("'(' expected");
    }
	nextToken();
	
	if(isNonKeyId(token) == false)
	{
		processError("NON_KEY_ID expected instead of: " + token);
	}
	
    listOfIds = ids();
	
    if (token != ")")
    {
        processError("',' or ')' expected after non-keyword identifier");
    }
    nextToken();
	
    if (token != ";")
    {
        processError("';' expected");
    }
	
	uint i = 0;
	while (temp != "")
	{
		temp = "";
		while(listOfIds[i] != ',' && i < listOfIds.length())
		{
			temp += listOfIds[i];
			i = i + 1;
		}
		i++;
		if (temp != "")
		{
			code("read",temp);
		}
	}
}
void Compiler::writeStmt()
{
    string x = "";
	nextToken();
	if (token != "(")
    {
        processError("'(' expected");
    }
    nextToken();
    string listOfIds = ids();
    for (uint i = 0; i < listOfIds.length(); i++)
    {
        if (listOfIds[i] == ',')
        {
            code("write",x);
            x = "";
        }
        else
        {
            x += listOfIds[i];
        }
    }
    code("write",x);
    if (token != ")")
    {
        processError("',' or ')' expected after non-keyword identifier");
    }
    nextToken();
	
    if (token != ";")
    {
        processError("';' expected");
    }
}
void Compiler::express()
{
	/*nextToken();
    if (token != "not" || !isBoolean(token) || 
		token != "(" || token != "+" || 
		token != "-" || isInteger(token) || 
		isNonKeyId(token))
    {
        processError("'not', '(', '+', '-', boolean literal, integer literal, or non-keyword identifier expected");
    }*/	
    term();
    expresses();
}
void Compiler::expresses()
{
    if (isRelOperator(token))
    {
        pushOperator(token);
        nextToken();
        term();
		
        string rightHandBound = popOperand();
        string leftHandBound = popOperand();
        code(popOperator(), rightHandBound, leftHandBound);
		expresses();
    }
}
void Compiler::term()
{
    factor();
    terms();
}
void Compiler::terms()
{
	if (isAddLevelOperator(token))
    {
        pushOperator(token);
        nextToken();
        factor();
		
        string rightHandBound = popOperand();
        string leftHandBound = popOperand();
		
        code(popOperator(), rightHandBound, leftHandBound);
        terms();
    }
}
void Compiler::factor()
{
    /*if (token != "not" || !isBoolean(token) || 
		token != "(" || token != "+" || 
		token != "-" || !isInteger(token) || 
		!isNonKeyId(token))
    {
        processError("'not', '(', '+', '-', boolean literal, integer literal, or non-keyword identifier expected");
    }*/
    part();
    factors();
}
void Compiler::factors()
{
	if (isMultLevelOperator(token))
    {
        pushOperator(token);
        nextToken();
        part();
		
        string rightHandBound = popOperand();
        string leftHandBound = popOperand();
        code(popOperator(), rightHandBound, leftHandBound);
		
        factors();
    }
}
void Compiler::part()
{
	if (token == "not")
    {
        nextToken();
        if (token == "(")
        {
			nextToken();
            express();
			
            if (token != ")")
            {
                processError("')' expected");
            }
            
            string rightHandBound = popOperand();
            code("not", rightHandBound);
			nextToken();
        }
        else if (isBoolean(token))
        {
            if (token == "true")
            {
                pushOperand("false");
            }
            else
            {
                pushOperand("true");
            }
            nextToken();
        }
        else if (isNonKeyId(token))
        {
			if (whichType(token) != BOOLEAN)
			{
				processError("type boolean expected for symbol");
			}
            code("not", token);                                                            //not sure how to do this part
            nextToken();
        }
        else
        {
            processError("'(', boolean literal, or non-keyword identifier expected");
        }
    }

    else if (token == "+")
    {
        nextToken();
        if (token == "(")
        {
			nextToken();
            express();
			
            if (token != ")")
            {
                processError("')' expected");
            }
            nextToken();
        }
		else if (token == "-")
		{
			nextToken();
			if (isInteger(token))
			{
				pushOperand("-" + token);
				nextToken();
			}
		}
        else if (isInteger(token) || isNonKeyId(token))
        {
            pushOperand(token);
            nextToken();
        }
        else
        {
            processError("expected '(', integer, or non-keyword id; found " + token);
        }
    }

    else if (token == "-")
    {
        nextToken();

        if (token == "(")
        {
			nextToken();
            express();
            if (token != ")")
            {
                processError("')' expected");
            }

            string rightHandBound = popOperand();
            code("neg", rightHandBound);
			
			nextToken();
        }
        else if (isInteger(token))
        {
            pushOperand("-" + token);
            nextToken();
        }
        else if (isNonKeyId(token))
        {
            code("neg", token);                                                   //not sure how to do this call
            nextToken();
        }
        else
        {
            processError("'(', integer literal, or non-keyword identifier expected");
        }
    }

	
	
    else if (token == "(")
    {
		nextToken();
        express();
        if (token != ")")
        {
            processError("')' expected");
        }
        nextToken();
    }

    else if (isBoolean(token) || isInteger(token) || isNonKeyId(token))
    {
        pushOperand(token);
        nextToken();
    }

	else
    {
        processError("'not', '(', '+', '-', boolean literal, integer literal, or non-keyword identifier expected");
    }
}
/**END STAGE 1 GRAMMAR FUNCTIONS**/



/**Stage 2 Grammar Functions**/
void Compiler::ifStmt()
{// stage 2, production 3
	if(token != "if")
	{
		processError("Keyword \"if\" expected instead of: " + token);
	}
	
	nextToken();
    express();
	
    if (token != "then")
    {
        processError("Keyword \"then\" expected instead of: " + token);
    }
	
    code("then",popOperand(), "");
	nextToken();
    execStmt();
	nextToken();
	if (token == "else")
	{
		code("else",popOperand(), "");
		nextToken();
		elsePt();
		string placeholder = popOperand();
		code("post_if",placeholder, "");
	}
	else
	{
		code("post_if",popOperand(), "");
	}
}
void Compiler::elsePt()
{// stage 2, production 4
    /*if(!isNonKeyId(token))
    {
        code("post_if",popOperand());
    }
    if(token == "else")
    {
        processError("Keyword \"else\" expected instead of: " + token);
        code("else",popOperand());
        execStmt();
        code("post_if",popOperand());
    }
    else if(isNonKeyId(token))
    {
        code("post_if",popOperand());
    }
    else
    {
        processError("expected: 'else' got: "+token);
    }*/
	execStmt();
}
void Compiler::whileStmt()
{// stage 2, production 5
    if(token != "while")
    {
        processError("expected: 'while' got: "+token);
    }
	
    code("while", "", "");
	nextToken();
    express();
	
    if(token != "do")
    {
        processError("expected: 'do' got: "+token);
    }
	
	code("do",popOperand(), "");
    nextToken();
    execStmt();
    
    code("post_while",popOperand(),popOperand());
}
void Compiler::repeatStmt()
{// stage 2, production 6
    if(token != "repeat")
    {
        processError("expected: 'repeat' got: "+token);
    }
	
    code("repeat", "", "");
    nextToken();
	execStmts();
    
    if(token != "until")
    {
        processError("expected: 'until' got: "+token);
    }
	
	nextToken();
    express();
	
	if(token != ";")
    {
        processError("expected: ';' got: "+token);
    }
	
	//must be done before code or else we run into segmentation faults
	string operand1 = popOperand();
	string operand2 = popOperand();
	
    code("until",operand1,operand2);
    nextToken();
    
}
void Compiler::nullStmt()
{// stage 2, production 7
    if(token != ";")
    {
        processError("expected: ';' got: "+token);
    }
    nextToken();
}
/*END STAGE 2 GRAMMAR FUNCTIONS*/


/**Pascallite Lexicon**/
bool Compiler::isKeyword(string s) const
{//UPDATED FOR STAGE 1
	if(s == "program" || s == "begin" || 
	   s == "end" ||  s == "var" || 
	   s == "const" || s == "integer" || 
	   s == "boolean" || s == "true" || 
	   s == "false" || s == "not" /*Stage 0 Keywords*/|| 
	   s =="mod" || s == "div" || 
	   s == "and" || s == "or" || 
	   s == "read" || s == "write" || s == ":=" || s == "<=" || s == ">=" || s == "<>" ||                             //Stage 1 Keywords
       s == "if" || s == "then" || s == "else" || s == "while" || s == "do" || s == "repeat" || s == "until")         //Stage 2 Keywords                                                         
	    return true;
    else
        return false;
}
bool Compiler::isSpecialSymbol(char c) const
{//UPDATED FOR STAGE 1
	if (c == ',' || c == ';' || 
	    c == '=' || c == '+' || 
		c == '-' || c == '.' || 
		c == '*' || c == '(' || 
		c == ')' || c == ':' ||
		c == '<' || c == '>')
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool Compiler::isNonKeyId(string s) const
{//not keywords, var names s[0] != num
	if (isKeyword(s) || isdigit(s[0]))
		return false;
	for(uint i = 0; i < s.length(); i++){
		if (!islower(s[i]) && !isdigit(s[i]) && s[i] != '_')		//not a keyword and not an ID
			return false;
	}
	return true;
}
bool Compiler::isInteger(string s) const
{
	for(uint i = 0; i < s.length(); i++)
    {
        if(!isdigit(s[i]))
            return false;
    }
	return true;
}
bool Compiler::isBoolean(string s) const
{
	if (s == "true" || s == "false")
    {
        return true;
    }
    return false;
}
bool Compiler::isLiteral(string s) const
{
    //INTEGER | BOOLEAN | 'not' BOOLEAN | '+' INTEGER | '-' INTEGER
	if(isInteger(s) || 
		isBoolean(s) || 
		(s.substr(0,3) == "not" && 
		isBoolean(s.substr(3,s.length()-1))) || 
		(s[0] == '+' && 
		isInteger(s.substr(1,s.length()-1))) || 
		(s[0] == '-' && isInteger(s.substr(1,s.length()-1))))
	{
        return true;
    }
    return false;
}
/**END PASCALLITE LEXICON**/

/**STAGE 1 PASCALLITE LEXICON `OUR` HELPER FUNCTIONS**/
bool isRelOperator(string name)
{
    if(name == "=" || name == "<>" || name == "<=" || name == ">=" || name == "<" || name == ">")
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool isAddLevelOperator(string name)
{
    if(name == "+" || name == "-" || name == "or")
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool isMultLevelOperator(string name)
{
    if(name == "*" || name == "div" || name == "mod" || name == "and")
    {
        return true;
    }
    else
    {
        return false;
    }
}
/*
bool locatedInSymbolTable(string name) //bc its not a Compiler:: function we cant call it the same way as we did in the others.
{//call to symbolTable, need to call through .h?
	//map<string, SymbolTableEntry>::iterator symTab;
	name = name.substr(0,15);
	//symTab = (symbolTable.count(name)); //fixme
    if (symTab == 1)    //that was a placeholder, use this
    {
        return true;
    }
    return false;

    //Ryan:
    for (uint i = 0; i < symbolTable.size(); i++) {
		if (symbolTable.at(i).externalName == name) {
			return true;
    
}
*/
/**END STAGE 1 PASCALLITE LEXICON HELPER FUNCTIONS**/


/**Action Routines**/
void Compiler::insert(string externalName, storeTypes inType, modes inMode, string inValue, allocation inAlloc, int inUnits)  
{//create symbol table entry for each identifier in list of external names
 //Multiply inserted names are illegal   
    string name = "";
	uint leftName = 0;
	
    while (leftName < externalName.length())
	{
		name = "";
		while(name == "")
		{
			while (leftName < externalName.length() && externalName[leftName] != ',')
			{
				name = name + externalName[leftName];
				leftName++;
			}
			leftName++;
			if (!name.empty())
			{
				if (name.length() > 15)
				{
					name = name.substr(0,15);
				}
				if(symbolTable.count(name) > 0)
				{
					processError("symbol " + name + " is multiply defined");
				}
				else if (isKeyword(name) && name != "true" && name != "false")
				{
					processError("illegal use of keyword");
				}
				else if(symbolTable.size() >= 256)
				{
					processError("overflow with too many non-keyword identifier declarations");
				}
				
				else
				{//time to create a table entry whoopee
			
					if(isupper(name[0]))
					{
						symbolTable.insert({name, SymbolTableEntry(name, inType, inMode, inValue, inAlloc, inUnits)});
					}
					else
					{
						symbolTable.insert({name,SymbolTableEntry(genInternalName(inType),inType,inMode,inValue,inAlloc,inUnits)});
					}
				}
				
			}
		}
	}
}
storeTypes Compiler::whichType(string name)		 
{//tells which data type a name has
	map<string, SymbolTableEntry>::iterator placeholder;
	placeholder = symbolTable.find(name);
	storeTypes dtype;
    if (isLiteral(name))
    {
        if (isBoolean(name))
        {
			dtype = BOOLEAN;
			return dtype;
		}
        else
		{
            dtype = INTEGER;
			return dtype;
		}
    }
    else		//name is an identifier and hopefully a constant
    {
        if (placeholder != symbolTable.end())
		{
            dtype = placeholder->second.getDataType();
		}
        else
		{
			
            processError("reference to undefined symbol " + name);
		}
    }
    return dtype;
}
string Compiler::whichValue(string name)		
{//tells which value a name has
	//map<string, SymbolTableEntry>::iterator placeholder;
    //placeholder = symbolTable.find(name);
	string value;
    if (isLiteral(name))
    {
        value = name;
    }
    else //name is an identifier and hopefully a constant
    {
        //if (symbolTable[name] != "" && symbolTable.getValue() != "")  NEW ATTEMPT
        if (symbolTable.count(name) > 0) //&& symbolTable.at(name).getValue() != "")
		{
            value = symbolTable.at(name).getValue();
        }
        else
        {
            processError("reference to undefined constant");	//fixme
        }
    }
    return value;
}
void Compiler::code(string op, string operand1, string operand2)
{//[if 'program': goto emitPrologue()  if 'end': goto emitEpilogue()]
    if (op == "program")
        emitPrologue(operand1,operand2);
    else if (op == "end")
        emitEpilogue(operand1,operand2);
    //**STAGE 1 ADDED CODE**//
    else if (op == "read")
		emitReadCode(operand1,operand2);
    else if (op == "write")
        emitWriteCode(operand1,operand2);
    else if (op == "+") // this must be binary '+'
        emitAdditionCode(operand1,operand2);
    else if (op == "-") // this must be binary '-'
        emitSubtractionCode(operand1,operand2);
    else if (op == "neg") // this must be unary '-'
        emitNegationCode(operand1,operand2);
    else if (op == "not")
        emitNotCode(operand1,operand2);
    else if (op == "*")
        emitMultiplicationCode(operand1,operand2);
    else if (op == "div")
        emitDivisionCode(operand1,operand2);
    else if (op == "mod")
        emitModuloCode(operand1,operand2);
    else if (op == "and")
        emitAndCode(operand1,operand2);
	else if (op == "or")
		emitOrCode(operand1, operand2);
    else if (op == "=")
        emitEqualityCode(operand1,operand2);
	else if (op == "!=" || op == "<>")
		emitInequalityCode(operand1, operand2);
    else if (op == ":=")
        emitAssignCode(operand1,operand2);
	else if (op == ">")
		emitGreaterThanCode(operand1, operand2);
	else if (op == ">=")
		emitGreaterThanOrEqualToCode(operand1, operand2);
	else if (op == "<")
		emitLessThanCode(operand1, operand2);
	else if (op == "<=")
		emitLessThanOrEqualToCode(operand1, operand2);
    else if(op == "then")
        emitThenCode(operand1, "");
    else if(op == "else")
        emitElseCode(operand1, "");
    else if(op == "post_if")
        emitPostIfCode(operand1, "");
    else if(op == "while")
        emitWhileCode("", "");
    else if(op == "do")
        emitDoCode(operand1, "");
    else if(op == "post_while")
        emitPostWhileCode(operand1,operand2);
    else if(op == "repeat")
        emitRepeatCode("", "");
    else if(op == "until")
        emitUntilCode(operand1,operand2);
    else
	{
		processError("compiler error since function code should not be called with illegal arguments");
	}
}
/**END ACTION ROUTINES**/



/**STAGE 1 ACTION ROUTINES & DATA**/
void Compiler::pushOperator(string op)
{
	operatorStk.push(op);
}
string Compiler::popOperator()
{
    string output = "";
    if(!operatorStk.empty())
	{
        output = operatorStk.top();
        operatorStk.pop();
		return output;
	}
	else
		processError("compiler error; operator stack underflow");
	return output;
}
void Compiler::pushOperand(string operand)
{
	if(isLiteral(operand) && symbolTable.count(operand) == 0)
	{
		if(operand == "true")
			symbolTable.insert({"true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, operand, YES, 1)});
		else if(operand == "false")
			symbolTable.insert({"false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, operand, YES, 1)});
		else
			insert(operand, INTEGER, CONSTANT, operand, YES, 1);
	}
	operandStk.push(operand);
}
string Compiler::popOperand()
{
    string output = "";
    if(!operandStk.empty())
	{
        output = operandStk.top();
        operandStk.pop();
		return output;
	}
	else
		processError("compiler error; operand stack underflow");
	return output;
}
/**END STAGE 1 ACTION ROUTINES & DATA**/



/**Emit Functions**/
void Compiler::emit(string label, string instruction, string operands, string comment)
{//prints objectFile + instruction + operands
    objectFile << left << setw(8) << label << setw(8) << instruction << setw(24) << operands << comment << endl;
}
void Compiler::emitPrologue(string progName, string)
{//prints objectFile + '%INCLUDE' + startup syntax
    time_t t = time(NULL);

    objectFile << "; Dylan McClellan, Michael Stillabower, Ryan Morris\t\t" << asctime(localtime(&t));
    //Output the %INCLUDE directives
    objectFile << "%INCLUDE \"Along32.inc\"" << endl;
    objectFile << "%INCLUDE \"Macros_Along.inc\"" << endl << endl;
    //Output identifying comments at beginning of objectFile
    emit("SECTION", ".text");
    emit("global", "_start", "", "; program " + progName.substr(0,15));
	objectFile << endl;
    emit("_start:");
}
void Compiler::emitEpilogue(string, string)
{//goto emitStorage()
    emit("","Exit", "{0}");
	objectFile << endl;
    emitStorage();
}
void Compiler::emitStorage()
{
    emit("SECTION", ".data", "", "");

    for (auto it = symbolTable.begin(); it != symbolTable.end(); it++)      //for loop the entries in the symbolTable that have an allocation of YES and a storage mode of CONSTANT
    {
        if (it->second.getAlloc() == YES && it->second.getMode() == CONSTANT)
        {
			string newValueToStore;
            if (it->second.getValue() == "true")
			{
                newValueToStore = "-1";
			}
            else if (it->second.getValue() == "false")
			{
                newValueToStore = "0";
			}
			else
			{
				newValueToStore = it->second.getValue();
			}
			
            emit(it->second.getInternalName(), "dd", newValueToStore, "; " + it->first);
        }
    }
	objectFile << endl;
    emit("SECTION", ".bss", "", "");

    for (auto it = symbolTable.begin(); it != symbolTable.end(); it++)      //those entries in the symbolTable that have an allocation of YES and a storage mode of VARIABLE
    {
        if (it->second.getAlloc() == YES && it->second.getMode() == VARIABLE)
        {
            emit(it->second.getInternalName(), "resd", "1", "; " + it->first);
        }
    }
}
/**END EMIT FUNCIONS**/



/**STAGE 1 EMIT FUNCTIONS & DATA**/
void Compiler::emitReadCode(string operand, string empty)
{
	map<string, SymbolTableEntry>::iterator placeholder;
	string name;
	while (operand.length() > 0)
	{
		uint index = operand.find(',');                              
		
		if (index != 0 && index <= operand.length())                 
		{                                                            
			name = operand.substr(0, index);                      	 
			operand = operand.substr(index + 1, operand.length());   
		}                                                            
		else                                                         
		{                                                            
			index = operand.length();                                
			name = operand.substr(0, index);                         
			uint colon = name.find(':');
			
			if (colon > 0 && colon < name.length())
			{
				name = name.substr(0, name.length() - 1);
			}
			operand = operand.substr(index, operand.length());       
		}
		placeholder = symbolTable.find(name);
		
		if (placeholder != symbolTable.end())
		{
			
		}
		else
		{
			processError("reference to undefined symbol");
		}
		if (placeholder->second.getDataType() != INTEGER)
		{
			processError("can't read variables of this type");
		}
		if (placeholder->second.getMode() != VARIABLE)
		{
			processError("attempting to read to a read-only location");
		}
		
		//call emits
		emit("", "call", "ReadInt", "; read int; value placed in eax");
		
		emit("", "mov", "[" + placeholder->second.getInternalName() + "],eax", "; store eax at " + placeholder->first);
		contentsOfAReg = name;
	}
}
/*{
	string name = "%";
	uint count = 0;
	while(name != "")
	{
		name = "";
		while(count < operand.length() && operand[count] != ',')
		{
			name += operand[count]; 
			count++; 
		}
		count++; 
		
		if(name == "")
			break;
		
		if(symbolTable.count(name) == 0)
			processError("reference to undefined symbol");
		
		
		
		if(symbolTable.at(name).getDataType() != INTEGER)
		{
			processError("Cant process anything other than INTEGER types");
		}
		if(symbolTable.at(name).getMode() != VARIABLE)
			processError("Trying to write to a read only location, smh");
		emit(" ", "call", "ReadInt", "; read int; value placed in eax");
		emit(" ", "mov", ("[" + symbolTable.at(name).getInternalName()+"],eax"), (";store eax at " + name));
		contentsOfAReg = name;
	}
}*/
void Compiler::emitWriteCode(string operand, string empty)
{
    string name = "%";
    uint count = 0;
    while (name!="")
    {
		name = "";
        while(operand[count] != ',' && count < operand.length())
        {//name is broken from list (operand) and put in name != ""
            name += operand[count];
			count = count + 1;
		}
		count++;
		if (name == "")
		{
			break;
		}
		if (symbolTable.count(name) == 0)
        {
            processError("reference to undefined symbol");
        }
        if (contentsOfAReg != name) //if name is not in the A register
        {
            emit("", "mov", "eax,[" + symbolTable.find(name)->second.getInternalName() + "]", "; load " + name + " in eax");
            contentsOfAReg = name;
        }
        if (symbolTable.find(name)->second.getDataType() == INTEGER || symbolTable.find(name)->second.getDataType() == BOOLEAN)
        {
            //emit code to call the Irvine WriteInt function
            emit("", "call", "WriteInt", "; write int in eax to standard out");
        }
		//emit code to call the Irvine Crlf function
		emit("", "call", "Crlf", "; write \\r\\n to standard out");
    }
}
void Compiler::emitAssignCode(string operand1, string operand2)
{//op2 = op1
    if(whichType(operand2) != whichType(operand1))     //types of operands are not the same
    {
        processError("incompatible types for operator ':='");
    }
    if(symbolTable.find(operand2)->second.getMode() != VARIABLE)      //storage mode of operand2 is not VARIABLE
    {
        processError("symbol on left-hand side of assignment must have a storage mode of VARIABLE");
    }
    if(operand1 == operand2)
    {
        return;
    }    
    if (operand1 != contentsOfAReg)
	{
        //then emit code to load operand1 into the A register IF the value is 0 or -1
		if (whichValue(operand1) == "-1" || whichValue(operand1) == "0")
		{
			emit("", "mov", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand1);
			contentsOfAReg = operand1;
		}
		else
		{
			emit("", "mov", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand1);
			contentsOfAReg = operand1;
		}
	}
	
	//emit code to store the contents of that register into the memory location pointed to by operand2
	emit("", "mov", "[" + symbolTable.find(operand2)->second.getInternalName() + "],eax", "; " + operand2 + " = AReg");
		
	//set the contentsOfAReg = operand2
	contentsOfAReg = operand2;

    //if operand1 is a temp then free its name for reuse
    if (isTemporary(operand1))
    {
        freeTemp();
    }
    //operand2 can never be a temporary since it is to the left of ':='
}
void Compiler::emitAdditionCode(string operand1, string operand2)
{//op2 +  op1
    operand1 = operand1.substr(0,15);
    operand2 = operand2.substr(0,15);
    if (whichType(operand2) != INTEGER || whichType(operand1) != INTEGER)
    {
        processError("binary '+' requires integer operands");
    }    
    if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))               //(A Register holds a temp not operand1 nor operand2)
    {
        //emit code to store that temp into memory
		emit("", "mov", "[" + symbolTable.find(contentsOfAReg)->second.getInternalName() + "],eax", "; deassign AReg");

        //change the allocate entry for the temp in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
		symbolTable.find(contentsOfAReg)->second.setUnits(1);
        
        //then deassign it
        contentsOfAReg == "";
    }
    if (!isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))               //(A register holds a non-temp not operand1 nor operand2)
    {
        //then deassign it
        contentsOfAReg == "";
    }
    if (contentsOfAReg != operand2 && contentsOfAReg != operand1) //(neither operand is in A register)
    {
        //then emit code to load operand2 into A register
		emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
    
    if (contentsOfAReg == operand1)
    {
		emit("", "add", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand1 + " + " + operand2);
    }
	else if (contentsOfAReg == operand2)
    {
		emit("", "add", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " + " + operand1);        
    }

    //deassign all temporaries involved in the addition and free those names for reuse
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }
    //A Register = next available temporary name and change type of its symbol table entry to integer
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);

    //push the name of the result onto operandStk
    pushOperand(contentsOfAReg);
}
void Compiler::emitSubtractionCode(string operand1, string operand2)
{//op2 -  op1
	if(whichType(operand1) != INTEGER || whichType(operand2) != INTEGER)
    {
		processError("illegal type within 'subtraction' operation");        
    }

	if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
    {
        //then emit code to store that temp into memory
		emit("", "mov","[" + symbolTable.find(contentsOfAReg)->second.getInternalName() + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
		symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
		symbolTable.find(contentsOfAReg)->second.setUnits(1);

        //deassign it
		contentsOfAReg = "";
	}
	if (contentsOfAReg != operand2)
    {
        //then emit code to load operand2 into the A register
		emit("", "mov","eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = operand2;
	}
	//emit code to perform register-memory subtraction
	if(contentsOfAReg == operand1)
    {
		emit("", "sub", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand1 + " - " + operand2);
    }
	else if(contentsOfAReg == operand2)
    {
		emit("", "sub", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " - " + operand1);        
    }
    
	//deassign all temporaries involved in the subtraction and free those names for reuse
	if (isTemporary(operand2))
	{
		freeTemp();
	}
	if (isTemporary(operand1))
	{
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}
void Compiler::emitNegationCode(string operand1, string empty)
{//-op1 "illegal type within 'negation' operation"
    if(whichType(operand1) != INTEGER)
    {
		processError("Illegal type for negation, must be type INTEGER");        
    }
	
	if(contentsOfAReg != operand1) 
	{
        //then emit code to load operand1 into the A register
		emit("","mov","eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]","; AReg = " + operand1);
		contentsOfAReg = operand1;
	}
    //emit code to perform a register invert with A register holding the result
	emit("","neg","eax","; AReg = -AReg");

	// deassign all temporary variables involved
	if (isTemporary(operand1))
    {
		freeTemp();
	}
	// A-reg now contains a new temporary variable, the result of the negation
	contentsOfAReg = getTemp();
	symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);
	
	// Push the new temporary variable onto the stack
	pushOperand(contentsOfAReg);
}
void Compiler::emitNotCode(string operand1, string empty)
{//!op1
    if (symbolTable.find(operand1)->second.getDataType() != BOOLEAN)
    {
        processError("illegal type within 'not' operation");
    }    

    if (contentsOfAReg != operand1)           //(neither operand is in A register)
    {
        //then emit code to load operand1 into the A register;
        emit("","mov","eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]","; AReg = " + operand1);
    }
    contentsOfAReg = operand1;

    //emit code to perform a register-memory inversion with A Register holding the result;
    emit("","not","eax","; AReg = !AReg");

    //deassign all temporaries involved and free those names for reuse;
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to boolean 
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);

    //push name of result onto operandStk;
    pushOperand(contentsOfAReg);
}
void Compiler::emitMultiplicationCode(string operand1, string operand2)
{//op2 *  op1
    if (whichType(operand2) != INTEGER || whichType(operand1) != INTEGER)
    {
        processError("illegal type within 'multiplication' operation");   
    }
    if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))      //(A Register holds a temp not operand1 nor operand2)
    {
        //emit code to store that temp into memory
        emit("", "mov", "[" + symbolTable.find(contentsOfAReg)->second.getInternalName() + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);

        //then deassign it
        contentsOfAReg = "";
    }
    if (!isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))
    {
        //then deassign it
        contentsOfAReg = "";
    }
    if (contentsOfAReg != operand2 && contentsOfAReg != operand1)       //(neither operand is in A register then)
    {
        //emit code to load operand2 into the A register;
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
    //emit code to perform a register-memory multiplication with A Register holding the result;
    if (contentsOfAReg == operand1)
    {
        emit("", "imul", "dword [" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand1 + " * " + operand2);	//	
    }
    else if (contentsOfAReg == operand2)
	{
        emit("", "imul", "dword [" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " * " + operand1);
    }

    //deassign all temporaries involved and free those names for reuse;
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to integer
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);

    //push name of result onto operandStk;
    pushOperand(contentsOfAReg);
}
void Compiler::emitDivisionCode(string operand1, string operand2) 
{//op2 /  op1
	if (whichType(operand2) != INTEGER || whichType(operand1) != INTEGER)       //(type of either operand is not integer)
    {
       processError("illegal type within 'division' operation");
    }
    if (operand1 == "0")
    {
        processError("ERROR: CANNOT DIVIDE BY ZERO");
    }
    if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)                                 //contentsOfAReg = temp     operand 2 = operand 2
    {
        //emit code to store that temp into memory
        emit("", "mov","[" + symbolTable.find(contentsOfAReg)->second.getInternalName() + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);

        //then deassign it
        contentsOfAReg = "";
    }
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)                             // if the A register holds a non-temp not operand2 
    {
        //then deassign it
        contentsOfAReg = "";
    }
    if (contentsOfAReg != operand2)                                                             //(operand2 is not in A register)
    {
        //emit instruction to do a register-memory load of operand2 into the A register;
        emit("", "mov","eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]",  "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
    //emit code to extend sign of dividend from the A register to edx:eax
    emit("","cdq","","; sign extend dividend from eax to edx:eax");
    
    //emit code to perform a register-memory division
    emit("","idiv", "dword [" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);

    //deassign all temporaries involved and free those names for reuse
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to integer 
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);

    //push name of result onto operandStk;
    pushOperand(contentsOfAReg);
}
void Compiler::emitModuloCode(string operand1, string operand2)
{//op2 %  op1
	if(whichType(operand2) != INTEGER || whichType(operand1) != INTEGER)
    {
        processError("illegal type within 'modulo' operation");
    }
    if (isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
    {
        //then emit code to store that temp into memory
        emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);

        //deassign it
        contentsOfAReg = "";
    }
    if (!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
    {
        //then deassign it
        contentsOfAReg = "";
    }
    if (contentsOfAReg != operand2)
    {
        //then emit code to load operand2 into the A register
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
    //emit code to extend sign of dividend from the A register to edx:eax
    emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");

    //emit code to perform a register-memory division
    if (contentsOfAReg == operand2)
    {
        emit("", "idiv", "dword [" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);
    }
	else
    {
        emit("", "idiv", "dword [" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand1 + " div " + operand2);
    }
    //exchange the quotient and remainder
    emit("", "xchg", "eax,edx", "; exchange quotient and remainder");

    //deassign all temporaries involved and free those names for reuse;
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    // Q-reg contains the remainder, so store the remainder in a temporary variable and load that variable to A-reg
	contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(INTEGER);
    pushOperand(contentsOfAReg);
}
void Compiler::emitAndCode(string operand1, string operand2)
{//op2 && op1
    if (whichType(operand2) != BOOLEAN || whichType(operand1) != BOOLEAN)
    {
        processError("binary 'and' requires boolean operands");
    }    
    if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))     //(A Register holds a temp not operand1 nor operand2)
    {
        //emit code to store that temp into memory
        emit("", "mov", "[" + symbolTable.find(contentsOfAReg)->second.getInternalName() + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
        
        //then deassign it
        contentsOfAReg = "";
    }  
    if (!isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))           //(A register holds a non-temp not operand2 nor operand1)
    {
        //then deassign it
        contentsOfAReg = "";
    } 
    if (contentsOfAReg != operand2 && contentsOfAReg != operand1)           //(neither operand is in A register)
    {
        //then emit code to load operand2 into the A register;
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    } 
    
    //emit code to perform a register-memory 'and' with A Register holding the result;
    if (contentsOfAReg == operand1)
    {
		emit("", "and", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand1 + " and " + operand2);        
    }
	else
    {
		emit("", "and", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " and " + operand1);        
    }

    //deassign all temporaries involved and free those names for reuse;
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to boolean 
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);

    //push name of result onto operandStk;
    pushOperand(contentsOfAReg);
}
void Compiler::emitOrCode(string operand1, string operand2)
{//op2 || op1
    if (whichType(operand2) != BOOLEAN || whichType(operand1) != BOOLEAN)       //(type of either operand is not boolean)
    {
        processError("illegal type within 'or' operation");
    }
    if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))           //(A Register holds a temp not operand1 nor operand2)
    {
        //emit code to store that temp into memory
        emit("", "mov", "[" + symbolTable.find(contentsOfAReg)->second.getInternalName() + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);
        
        //then deassign it
        contentsOfAReg = "";
    }
    if (!isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))           //(A register holds a non-temp not operand2 nor operand1)
    {
        //then deassign it
        contentsOfAReg = "";
    }
    if (contentsOfAReg != operand2 && contentsOfAReg != operand1)               //(neither operand is in A register)
    {
        //then emit code to load operand2 into the A register;
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
    
    //emit code to perform a register-memory addition with A Register holding the result;
    if (operand1 == contentsOfAReg)
    {
        emit("", "or", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand1 + " or " + operand2);
    }
    else
    {
        emit("", "or", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; AReg = " + operand2 + " or " + operand1);
    }

    //deassign all temporaries involved and free those names for reuse;
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to boolean 
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);

    //push name of result onto operandStk;
    pushOperand(contentsOfAReg);
}
void Compiler::emitEqualityCode(string operand1, string operand2)
{//op2 == op1
    operand1 = operand1.substr(0,15);
    operand2 = operand2.substr(0,15);
    if (whichType(operand2) != whichType(operand1))           //(types of operands are not the same)
    {
        processError("incompatible types within 'equality' comparison");
    }
    if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))           //(the A Register holds a temp not operand1 nor operand2)
    {
        //then emit code to store that temp into memory
        emit("", "mov", "[" + symbolTable.find(contentsOfAReg)->second.getInternalName() + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);

        //deassign it
        contentsOfAReg = "";
    }
    if (!isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))           //(the A register holds a non-temp not operand2 nor operand1)
    {
        //then deassign it
        contentsOfAReg = "";
    }
    if (contentsOfAReg != operand2 && contentsOfAReg != operand1)           //(neither operand is in the A register)
    {
        //then emit code to load operand2 into the A register
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
    
    string lastLabel = getLabel();
	string Label = getLabel();

    //emit code to perform a register-memory compare
    if (contentsOfAReg == operand2)
	{
        emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
    }
	else
	{
        emit("", "cmp", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
    }
    //emit code to jump if equal to the next available Ln (call getLabel) 
    emit("", "je", lastLabel, "; if " + operand2 + " = " + operand1 + " then jump to set eax to TRUE");

    //emit code to load FALSE into the A register
    emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

    //insert FALSE in symbol table with value 0 and external name false
    if (symbolTable.find("false") == symbolTable.end())
    {
		symbolTable.insert({"false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});        
    }

    //emit code to perform an unconditional jump to the next label (call getLabel should be L(n+1))
    emit("", "jmp", Label, "; unconditionally jump");
    //emit code to label the next instruction with the first acquired label Ln
    emit(lastLabel + ":");
    //emit code to load TRUE into A register
    emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
    //insert TRUE in symbol table with value -1 and external name true
    if (symbolTable.find("true") == symbolTable.end() || symbolTable.find("TRUE") == symbolTable.end())
    {
        symbolTable.insert({"true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
    }
    //emit code to label the next instruction with the second acquired label L(n+1)
    emit((Label) + ":");
    //deassign all temporaries involved and free those names for reuse
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }
    //A Register = next available temporary name and change type of its symbol table entry to boolean
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);
    //push the name of the result onto operandStk
    pushOperand(contentsOfAReg);
}
void Compiler::emitInequalityCode(string operand1, string operand2)
{//op2 != op1
    operand1 = operand1.substr(0,15);
    operand2 = operand2.substr(0,15);
    if (whichType(operand2) != whichType(operand1))
    {
        processError("both types must be of type INTEGER for the '<=' operator");
    }
    if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))           //(the A Register holds a temp not operand1 nor operand2)
    {
        //then emit code to store that temp into memory
        emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);

        //deassign it
        contentsOfAReg = "";
    }
    if (!isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))           //(the A register holds a non-temp not operand2 nor operand1)
    {
        //then deassign it
        contentsOfAReg = "";
    }
    if (contentsOfAReg != operand2 && contentsOfAReg != operand1)
    {
        //then emit code to load operand2 into the A register
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
    //emit code to perform a register-memory compare with A register holding the result
    string lastLabel = getLabel();
	string Label = getLabel();
	
    if (contentsOfAReg == operand2)
    {
		emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
    }
	else
    {
        emit("", "cmp", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
    }
	
    emit("", "jne", lastLabel, "; if " + operand2 + " <> " + operand1 + " then jump to set eax to TRUE");
    //emit code to load FALSE into the A register
    emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

    //insert false into the symbol table if the last entry is false
    if (symbolTable.find("false") == symbolTable.end() || symbolTable.find("FALSE") == symbolTable.end())
    {
		symbolTable.insert({"false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});
    }

    emit("", "jmp", Label, "; unconditionally jump");
	emit( lastLabel + ":","", "", "");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

    //insert true into the symbol table if the last entry is true
    if (symbolTable.find("true") == symbolTable.end() || symbolTable.find("TRUE") == symbolTable.end())
    {
        symbolTable.insert({"true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
    }
	emit(Label + ":");

    //deassign all temporaries involved and free those names for reuse
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to boolean
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);

    //push the name of the result onto operandStk
    pushOperand(contentsOfAReg);
}
void Compiler::emitLessThanCode(string operand1, string operand2)
{//op2 <  op1
    operand1 = operand1.substr(0,15);
    operand2 = operand2.substr(0,15);
    if (whichType(operand2) != whichType(operand1))
    {
        processError("both types must be integers for the '<' operator");
    }

    if(isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))
    {
        //then emit code to store that temp into memory
        emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);

        //deassign it
        contentsOfAReg = "";
    }

    if (contentsOfAReg != operand2)
    {
        //then emit code to load operand2 into the A register
		contentsOfAReg = "";
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
	
    //emit code to perform a register-memory compare with A register holding the result
    string lastLabel = getLabel();
	string Label = getLabel();

    emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; compare " + operand2 + " and " + operand1);

    //emit("", "cmp", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; compare " + operand1 + " and " + operand2);

    emit("", "jl", lastLabel, "; if " + operand2 + " < " + operand1 + " then jump to set eax to TRUE");
    //emit code to load FALSE into the A register
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

    //insert false into the symbol table if the last entry is false
    if (symbolTable.find("false") == symbolTable.end() || symbolTable.find("FALSE") == symbolTable.end())
    {
		symbolTable.insert({"false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});        
    }
	
	emit("", "jmp", Label, "; unconditionally jump");
	emit(lastLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
    //insert true into the symbol table if the last entry is true
	if (symbolTable.find("true") == symbolTable.end() || symbolTable.find("TRUE") == symbolTable.end())
    {
		symbolTable.insert({"true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});        
    }
	emit(Label + ":");

    //deassign all temporaries involved and free those names for reuse
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to boolean
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);

    //push the name of the result onto operandStk
    pushOperand(contentsOfAReg);
}
void Compiler::emitLessThanOrEqualToCode(string operand1, string operand2)
{//op2 <= op1
    operand1 = operand1.substr(0,15);
    operand2 = operand2.substr(0,15);
    if (whichType(operand2) != whichType(operand1))
    {
        processError("both types must be of type INTEGER for the '<=' operator");
    }
    if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))
    {
        //then emit code to store that temp into memory
        emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);

        //deassign it
        contentsOfAReg = "";
    }
    if (!isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))
    {
        //then deassign it
        contentsOfAReg = "";
    }
    if (contentsOfAReg != operand2 && contentsOfAReg != operand1)
    {
        //then emit code to load operand2 into the A register
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
    //emit code to perform a register-memory compare with A register holding the result
	string lastLabel = getLabel();
	string Label = getLabel();

	if (contentsOfAReg == operand2)
    {
		emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
    }
	else
    {
		emit("", "cmp", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
    }

    emit("", "jle", lastLabel, "; if " + operand2 + " <= " + operand1 + " then jump to set eax to TRUE");
    //emit code to load FALSE into the A register
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	
    //insert false into the symbol table if the last entry is false
	if (symbolTable.find("false") == symbolTable.end() || symbolTable.find("FALSE") == symbolTable.end())
    {
		symbolTable.insert({"false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});        
    }
	
	emit("", "jmp", Label, "; unconditionally jump");
	emit(lastLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
    //insert true into the symbol table if the last entry is true
	if (symbolTable.find("true") == symbolTable.end() || symbolTable.find("TRUE") == symbolTable.end())
    {
        symbolTable.insert({"true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
    }
	emit(Label + ":");

    //deassign all temporaries involved and free those names for reuse
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to boolean
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);

    //push the name of the result onto operandStk
    pushOperand(contentsOfAReg);
}
void Compiler::emitGreaterThanCode(string operand1, string operand2)
{//op2 >  op1
    operand1 = operand1.substr(0,15);
    operand2 = operand2.substr(0,15);
    if (whichType(operand2) != whichType(operand1))
    {
        processError("both types must be of type INTEGER for the '>' operator");
    }
    if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))
    {
        //then emit code to store that temp into memory
        emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);

        //deassign it
        contentsOfAReg = "";
    }
	
    if (contentsOfAReg != operand2 && contentsOfAReg != operand1)
    {
        //then emit code to load operand2 into the A register
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
	
    //emit code to perform a register-memory compare with A register holding the result
	string lastLabel = getLabel();
	string Label = getLabel();

    if (contentsOfAReg == operand2)
    {
        emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
    }
	else
    {
        emit("", "cmp", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
    }
	
	emit("", "jg", lastLabel, "; if " + operand2 + " > " + operand1 + " then jump to set eax to TRUE");
	//emit code to load FALSE into the A register
    emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	
    //insert false into the symbol table if the last entry is false
	if (symbolTable.find("false") == symbolTable.end() || symbolTable.find("FALSE") == symbolTable.end())
    {
        symbolTable.insert({"false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});
    }
	
	emit("", "jmp", Label, "; unconditionally jump");
	emit(lastLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
    //insert true into the symbol table if the last entry is true
	if (symbolTable.find("true") == symbolTable.end() || symbolTable.find("TRUE") == symbolTable.end())
    {
        symbolTable.insert({"true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});
    }
	emit(Label + ":");

    //deassign all temporaries involved and free those names for reuse
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to boolean
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);

    //push the name of the result onto operandStk
    pushOperand(contentsOfAReg);
}
void Compiler::emitGreaterThanOrEqualToCode(string operand1, string operand2)
{//op2 >= op1
    operand1 = operand1.substr(0,15);
    operand2 = operand2.substr(0,15);
    if (whichType(operand2) != whichType(operand1))
    {
        processError("both types must be of type INTEGER for the '>=' operator");
    }
    if (isTemporary(contentsOfAReg) && (contentsOfAReg != operand2 && contentsOfAReg != operand1))
    {
        //then emit code to store that temp into memory
        emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");

        //change the allocate entry for it in the symbol table to yes
        symbolTable.find(contentsOfAReg)->second.setUnits(1);
        symbolTable.find(contentsOfAReg)->second.setAlloc(YES);

        //deassign it
        contentsOfAReg = "";
    }

    if (contentsOfAReg != operand2 && contentsOfAReg != operand1)
    {
        //then emit code to load operand2 into the A register
        emit("", "mov", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand2;
    }
    //emit code to perform a register-memory compare with A register holding the result
    string lastLabel = getLabel();
	string Label = getLabel();
	
	if (contentsOfAReg == operand2)
    {
		emit("", "cmp", "eax,[" + symbolTable.find(operand1)->second.getInternalName() + "]", "; compare " + operand2 + " and " + operand1);        
    }
	else
    {
		emit("", "cmp", "eax,[" + symbolTable.find(operand2)->second.getInternalName() + "]", "; compare " + operand1 + " and " + operand2);        
    }

	emit("", "jge", lastLabel, "; if " + operand2 + " >= " + operand1 + " then jump to set eax to TRUE");
    //emit code to load FALSE into the A register
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	
    //insert false into the symbol table if the last entry is false
	if (symbolTable.find("false") == symbolTable.end() || symbolTable.find("FALSE") == symbolTable.end())
    {
		symbolTable.insert({"false", SymbolTableEntry("FALSE", BOOLEAN, CONSTANT, "0", YES, 1)});        
    }
	
	emit("", "jmp", Label, "; unconditionally jump");
	emit(lastLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	
    //insert true into the symbol table if the last entry is true
	if (symbolTable.find("true") == symbolTable.end() || symbolTable.find("TRUE") == symbolTable.end())
    {
		symbolTable.insert({"true", SymbolTableEntry("TRUE", BOOLEAN, CONSTANT, "-1", YES, 1)});        
    }
	emit(Label + ":");

    //deassign all temporaries involved and free those names for reuse
    if (isTemporary(operand2))
    {
        freeTemp();
    }
    if (isTemporary(operand1))
    {
        freeTemp();
    }

    //A Register = next available temporary name and change type of its symbol table entry to boolean
    contentsOfAReg = getTemp();
    symbolTable.find(contentsOfAReg)->second.setDataType(BOOLEAN);

    //push the name of the result onto operandStk
    pushOperand(contentsOfAReg);
}
/**END STAGE 1 EMIT FUNCTIONS & DATA**/



/**Stage 2 Emit Functions & Data**/
void Compiler::emitThenCode(string operand1, string empty) //used 203 example
{
    // emit code which follows 'then' and statement predicate
    string tempLabel;
	
    //if the type of operand1 is not boolean
    if(symbolTable.at(operand1).getDataType() != BOOLEAN)
    {
        processError("if predicate must be of type boolean");
    }
	
    // assign next label to tempLabel
    tempLabel = getLabel();
	
    //if operand1 is not in the A register then
    if(contentsOfAReg != operand1)
    {
        //emit instruction to move operand1 to the A register
		contentsOfAReg = "";
        emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
        contentsOfAReg = operand1;
    }

    //emit instruction to compare the A register to zero (false)
    emit("", "cmp", "eax,0", "; compare eax to 0"); 
    
    //emit code to branch to tempLabel if the compare indicates equality
    emit("", "je", tempLabel, "; if " + operand1 + " is false then jump to end of if");
    
    //push tempLabel onto operandStk so that it can be refernced when emitElseCode() or emitPostIfCode() is called 
    pushOperand(tempLabel);

    if(isTemporary(operand1)) //if operand1 is a temp then
    {
        freeTemp();//free operand's name for reuse
    }

    //deassign operands from all registers
    contentsOfAReg = "";
}
void Compiler::emitElseCode(string operand1, string empty) //used 203 example
{
    // emit code which follows 'else' clause of 'if' statement
    string tempLabel;

    //assign next label to tempLabel
    tempLabel = getLabel();

    //emit instruction to branch unconditionally to tempLabel
    emit("", "jmp", tempLabel, "; jump to end if");

    //emit instruction to label this point of object code with the argument operand1
    emit(operand1 + ":", "", "", "; else");

    //push tempLabel onto operandStk
    pushOperand(tempLabel);

    //deassign operands from all registers
    contentsOfAReg = "";
}
void Compiler::emitPostIfCode(string operand1, string empty) //used 203 example
{
    // emit code which follows end of 'if' statement
    //emit instruction to label this point of object code with the argument operand1
    emit(operand1 + ":","","","; end if");

    //deassign operands from all registers
    contentsOfAReg = "";
}
void Compiler::emitWhileCode(string empty, string emptyagain) //used 202 example
{
    // emit code following 'while'
    string tempLabel;

    //assign next label to tempLabel
    tempLabel = getLabel();
    
    //emit instruction to label this point of object code as tempLabel
    emit(tempLabel + ":", "", "", "; while");

    //push tempLabel onto operandStk
    pushOperand(tempLabel);

    //deassign operands from all registers
    contentsOfAReg = "";
}
void Compiler::emitDoCode(string operand1, string empty) //used 202 example
{
    // emit code following 'do'
    string tempLabel;
    
    if (symbolTable.at(operand1).getDataType() != BOOLEAN)         //the type of operand1 is not boolean
    {
        processError("while predicate must be of type boolean");
    }
    
    //assign next label to tempLabel
    tempLabel = getLabel();

    if (contentsOfAReg != operand1)     //operand1 is not in the A register then
    {
        //emit instruction to move operand1 to the A register
		contentsOfAReg = "";
        emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
        contentsOfAReg = operand1;
    }
    
    //emit instruction to compare the A register to zero (false)
    emit("", "cmp", "eax,0", "; compare eax to 0"); 
    
    //emit code to branch to tempLabel if the compare indicates equality
    emit("", "je", tempLabel, "; if " + operand1 + " is false then jump to end while");
    
    //push tempLabel onto operandStk
    pushOperand(tempLabel);
    
    if (isTemporary(operand1))
    {
        //then free operand's name for reuse
        freeTemp();
    }
    
    //deassign operands from all registers
    contentsOfAReg = "";
}
void Compiler::emitPostWhileCode(string operand1, string operand2) //used 202 example
{
    // emit code at end of 'while' loop;
    // operand2 is the label of the beginning of the loop
    // operand1 is the label which should follow the end of the loop
	
    //emit instruction which branches unconditionally to the beginning of the loop, i.e., to the value of operand2
    emit("","jmp", operand1, "; end while");

    //emit instruction which labels this point of the object code with the argument operand1
    emit(operand2 + ":", "", "", "");

    //deassign operands from all registers
    contentsOfAReg = "";
}
void Compiler::emitRepeatCode(string empty, string emptyagain) //used 208 example
{
    // emit code which follows 'repeat'
    string tempLabel;

    //assign next label to tempLabel
    tempLabel = getLabel();

    //emit instruction to label this point in the object code with the value of tempLabel
    emit(tempLabel + ":", "", "", "; repeat");

    //push tempLabel onto operandStk
    pushOperand(tempLabel);

    //deassign operands from all registers
    contentsOfAReg = "";
}
void Compiler::emitUntilCode(string operand1, string operand2) //used 208 example
{
    // emit code which follows 'until' and the predicate of loop
    // operand1 is the value of the predicate
    // operand2 is the label which points to the beginning of the loop
    if (symbolTable.find(operand1)->second.getDataType() != BOOLEAN)
    {
        processError("if predicate must be of type boolean");
    }
        
    if (operand1 != contentsOfAReg)				//operand1 is not in the A register 
    {
        //then emit instruction to move operand1 to the A register
		contentsOfAReg = "";
        emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2);
        contentsOfAReg = operand1;
    }

	string label = getLabel();

    //emit instruction to compare the A register to zero (false)
    //emit("", "cmp", "eax,[" + label + "]", "; compare" + label + "and 0"); 
	emit(" ", "cmp", "eax,0", "; compare eax to 0"); 

    //emit code to branch to operand2 if the compare indicates equality
    emit("", "je", operand2, "; until " + operand1 + " is true");
    
    if (isTemporary(operand1))
    {
        //free operand1's name for reuse
        freeTemp();
    }
    
    //deassign operands from all registers
    contentsOfAReg = "";
}
/**END STAGE 2 EMIT FUNCTIONS & DATA**/



/**Lexical Routines**/
char Compiler::nextChar()                                              
{//returns the next character or end of file marker
    ch = sourceFile.get();
    if (sourceFile.eof())
    {
        ch = END_OF_FILE;     // eof == '$'
    }
    else
    {	
		//chToHold is a global variable for keeping track of the last char that we had
		//before exiting the function
        if(chToHold == '\n')
		{
			listingFile << right << setw(5) << ++lineNo << "|";	
		}
		listingFile << ch;
	}
	chToHold = ch;
    return ch;
}
string Compiler::nextToken()
{//returns the next token or end of file marker
	//char pre_ch;
    token = "";
    while (token == "")
    {   
		if(ch == '{')
		{
			//process comment
			while (ch != '}')
			{//nextChar() is not one of END_OF_FILE, '}'
				if (ch == END_OF_FILE)
					processError("unexpected end of file");
				else 
					nextChar();
			}
			if(ch == '}')
			{
				nextChar();
				token == "";
			}
		}
		else if(ch == '}')
		{
			processError("'}' cannot begin token");
		}
		else if(isspace(ch))
		{
			nextChar();
		}
		else if(isSpecialSymbol(ch))
		{
			token = ch;
			string concatenator = token;
			
            nextChar();
			
			if (isKeyword(concatenator + ch))
			{
				//we have found a special symbol that exists past the =, <, >, etc.
				concatenator = token;
				token += ch;
				
				//proceed the char
				nextChar();
			}
		}
		else if(islower(ch))
		{
			token = ch;
			nextChar();
			while ((isdigit(ch) || islower(ch)|| ch == '_')/* && ch != END_OF_FILE*/)
			{//nextChar() is one of letter, digit, or '_' but not END_OF_FILE
				if(token.back() == '_' && ch == '_')
				{
					processError("detected illegal use of multiple underscores");
				}
				token+=ch;
				nextChar();
			}	
			if (ch == END_OF_FILE)//ch is END_OF_FILE
				processError("unexpected end of file");
		}
		else if(isdigit(ch)){
			token = ch;
			nextChar();
			while (isdigit(ch) && ch != END_OF_FILE)
			{//nextChar() is digit but not END_OF_FILE*
				token+=ch;
				nextChar();
			}
			if (ch == END_OF_FILE)//ch is END_OF_FILE
				processError("unexpected end of file");
		}
		else if(ch == END_OF_FILE){
			token = ch;
		}
		else{
			processError("illegal symbol");
		}
	}
	token = token.substr(0,15);
    return token;
}
/**END LEXICAL ROUTINES**/

/**Other Routines**/
string Compiler::genInternalName(storeTypes stype) const
{
    static int boolCount;
	static int intCount;
	static int progCount;
    string name;
    if(stype == BOOLEAN)
    {
        name = "B";
		name += to_string(boolCount);
        boolCount++;
    }
    if(stype == INTEGER)
    {
        name = "I";
		name += to_string(intCount);
        intCount++;
    }
    if(stype == PROG_NAME)
    {
        name = "P";
		name += to_string(progCount);
        progCount++;
    }
	return name;
}
void Compiler::processError(string err)
{//Output err to listingFile
	errorCount++;
	listingFile<< endl << "Error: Line " << lineNo << ": " << err << endl;
	createListingTrailer();
    //Call exit() to terminate program
	exit(0);
}
/**END OTHER ROUTINES**/

/**STAGE 1 OTHER ROUTINES**/
void Compiler::freeTemp()
{
    currentTempNo--;
    if (currentTempNo < -1)
    {
        processError("compiler error, currentTempNo should be ≥ –1");
    }  
}
string Compiler::getTemp()
{
    string temp;
    currentTempNo++;
    temp = "T" + to_string(currentTempNo);
    if (currentTempNo > maxTempNo)
    {
        insert(temp, UNKNOWN, VARIABLE, "", NO, 1);
        maxTempNo++;
    }
    return temp;
}
string Compiler::getLabel()
{
    static int labelCount;
    string l = ".L";
    l += to_string(labelCount++);
	return l;
}
bool Compiler::isTemporary(string s) const
{//determines if s represents a temporary
    if(s[0]!='T')
        return false;
    return true;
} 
/**END STAGE 1 OTHER ROUTINES**/