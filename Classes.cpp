//
// Created by UPDATE on 17/05/2020.
//

#include "Classes.h"


extern int yylineno;
int flag = 0;


stack<Scope<Var>>* Scopes;
Scope<Func>* funcs;
unordered_map<string,string>* varsHash;
stack<int>* offsetsStack;

void init()
{
	Scopes = new stack<Scope<Var>>();
	funcs = new Scope<Func>();
	varsHash = new unordered_map<string,string>();
	offsetsStack = new stack<int>();
	offsetsStack->push(0);
	funcs->Elements.push_back(Func("print","VOID",vector<string>(1,"STRING")));
	funcs->Elements.push_back(Func("printi","VOID",vector<string>(1,"INT")));
	(*varsHash)["print"]="VOID";
	(*varsHash)["printi"]="VOID";
}

void destroy()
{
	delete Scopes;
	delete funcs;
	delete varsHash;
	delete offsetsStack;
	exit(0);
}


void openScope()
{
   //Scope<Var>* newScope = new Scope<Var>();
   Scopes->push(Scope<Var>());
   offsetsStack->push(offsetsStack->top());
}


void closeVarScope()
{
    output::endScope();
    Scope<Var> stackTop = Scopes->top();
    for(auto itr = stackTop.Elements.begin() ; itr != stackTop.Elements.end() ; itr++)
    {
        output::printID((itr)->name,(itr)->offset,(itr)->type);
		varsHash->erase((itr)->name);
    }
    //delete stackTop;
    Scopes->pop();
    offsetsStack->pop();
}

bool mainExists()
{
    for (auto i = funcs->Elements.begin(); i!=funcs->Elements.end(); ++i) {
        if (i->name == "main" && i->args.empty() && i->retType == "VOID")
			return true;
    }
    return false;
}

void closeFuncScope()
{
	if(!mainExists()){
		output::errorMainMissing();
		destroy();
	}
    output::endScope();
    for(auto itr = funcs->Elements.begin() ; itr != funcs->Elements.end() ; itr++)
    {
       output::printID((itr)->name,0, output::makeFunctionType((itr)->retType,(itr)->args));
    }
}


void insertVar(string name , string type)
{
    auto itr = varsHash->find(name);
    if(itr == varsHash->end())
    {
		//Var* addVar = new Var(name,type,offsetsStack->top());
		int i = offsetsStack->top();
		Var v(name,type,i);
		Scope<Var> s = Scopes->top();
		s.Elements.push_back(v);
		Scopes->pop();
		Scopes->push(s);
        (*varsHash)[name] = type;
    }
    else{
	  output::errorDef(yylineno,name);
	  destroy();
   }
   int i = offsetsStack->top()+1;
   offsetsStack->pop();
   offsetsStack->push(i);
}

void insertFunc(string name ,string retType, vector<string>& args , vector<string>& names)
{
	auto it = varsHash->find(name);
	if(it == varsHash->end()){
		funcs->Elements.push_back(Func(name,retType,args));
        (*varsHash)[name] = retType;
		int i = -1;
		auto itr1 = names.begin();
		for(auto itr = args.begin()  ; itr != args.end() ; itr++)
		{
		   //Var* addVar = new Var(*itr1,*itr,i);
		   Scopes->top().Elements.push_back(Var(*itr1,*itr,i));
		   varsHash->insert({(*itr1),(*itr)});
		   itr1++;
		   i--;
		}
	}
	else{
	  output::errorDef(yylineno,name);
	  destroy();
	}
}


string checkCall(vector<string> types , string name)
{
	auto it = funcs->Elements.begin();
	for(; it!= funcs->Elements.end(); it++)
	{
		if((it)->name==name) break;
	}
	if(it == funcs->Elements.end()){
		output::errorUndefFunc(yylineno,name);
		destroy();
	}
	if(types.size()!=it->args.size()){
		output::errorPrototypeMismatch(yylineno,name,it->args);
		destroy();
	}
	auto j = types.begin();
	for(auto i = it->args.begin(); i!=it->args.end(); i++){
		if(*i!=*j&&((*j)!="BYTE" || (*i)!="INT")){
			output::errorPrototypeMismatch(yylineno,name,it->args);
			destroy();
		}
		j++;
	}
	return it->retType;
}


void inLoop(){
    flag++;; // flag is a global var
}


void outLoop()
{
    flag--;
}

string idType(string id)
{
	auto it = varsHash->find(id);
	if(varsHash->end()==it){
		output::errorUndef(yylineno,id);
		destroy();
	}
	return it->second;
}

void checkLoop(int i)
{
	if(flag) return;
    if(i == 0 )
    {
        output::errorUnexpectedBreak(yylineno);
        exit(0);
    }

    else if(i==1)
    {
        output::errorUnexpectedContinue(yylineno);
        exit(0);
    }

}

void TypeError(){
	output::errorMismatch(yylineno);
	destroy();
}


void checkReturnType(string type)
{
    string t = funcs->Elements.back().retType;
    if( t == type || (t == "INT" && type == "BYTE") ) return; 
    output::errorMismatch(yylineno);
	destroy();
}

void checkByte(string num)
{
    if(stoi(num) <= 255) return;
	output::errorByteTooLarge(yylineno,num);
	destroy();
}

void checkRelOp(string* t1, string* t2)
{
	bool f = (((*t1)=="INT" || (*t1)=="BYTE") && ((*t2)=="INT" || (*t2)=="BYTE"));
	delete t1; delete t2;
	if(!f) TypeError();
}


