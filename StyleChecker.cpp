#include "clang/Analysis/CFG.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/Type.h"
#include "clang/AST/Decl.h"
#include "llvm/IR/Type.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/ASTContext.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <set>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

//Global Variables
std::set<std::string> changedVarNames;
int for_flag = 0;	//0 if not in a for loop, 1 if it is in a for loop.
int commentCount=0;


// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

class StyleVisitor : public RecursiveASTVisitor<StyleVisitor> {
  ASTContext *_context;
  Rewriter *_rewriter;
public:
  explicit StyleVisitor(ASTContext *context) : _context(context) {
    LangOptions LO;
    _rewriter = new Rewriter(_context->getSourceManager(),LO);
  }

  ~StyleVisitor(){
    _rewriter->overwriteChangedFiles();
    delete _rewriter;
  }

  //In here we want to do 2 thins:
  //1. Count how many //'s there are, ie how many comments.
  //still needs to be able to support libraries.
  bool VisitTranslationUnitDecl(TranslationUnitDecl *transDecl){
	if(!transDecl)
	      return true;
	LangOptions LO;
	PrintingPolicy policy = PrintingPolicy(LO);
	policy.adjustForCPlusPlus();
	transDecl->print(llvm::outs(), policy);
	return true;
  }

  //this is the part of the code that'll count comments
  //currently cannot detect any comments, not 100 percent sure why, but 
  //since comments are just not part of the AST not sure how to fix
  bool VisitDecl(Decl *decl){
	  if(!decl)
		return true;

	  ASTContext &context = decl->getASTContext();
	  const clang::Preprocessor *PP = nullptr;
	  comments::FullComment *comment = context.getCommentForDecl(decl, PP);
	  if(!comment)
		return true;
	
	  commentCount++;
	  llvm::errs() << commentCount << " Comments counted.\n";
	  return true;
  }

  bool VisitVarDecl(VarDecl *varDecl){
	  //conventions -- no lowercase letter l or capital letter O, likewise no numbers 1,0
	  //No single letter variables or constant(EXCEPT for LOOPS & FORS)
	  if(varDecl && for_flag == 0){ 
	  	std::string variableName = varDecl->getNameAsString();
	  	if(variableName.length()<2){
			llvm::errs() << "WARNING-Variable: " << variableName 
				<< " is 1 character. Consider lengthening. \n";
	  	}
	  	else if(variableName.length()>15){
		  	llvm::errs() << "WARNING-Variable: " << variableName 
				<< " is over 15 characters. Consider Shortening. \n"; 
	  	}
		//consider using string.find_first_of() -- replace for loop
		for(int i = 0; i<int(variableName.length());i++){
			char inspect = variableName[i];
			if(inspect =='1'||inspect =='0'||inspect =='l'||inspect=='O'){
				llvm::errs() << "WARNING-Variable: " << variableName << " contains "<< inspect 
					<<". Consider changing name. \n";
				break;
			}
		}
		QualType qualType = varDecl->getType();
		if(qualType.isConstQualified()){
			int flag = 0;
			char yesOrNo = 'N';
			for(int i = 0; i<int(variableName.length()); i++){
				if(variableName[i] >= 'a' && variableName[i]<='z' && flag == 0){
					llvm::errs() << "WARNING-Const variable: "
						<< variableName << " not all caps. Revise?(y/n)";
					std::cin >> yesOrNo;
					if(yesOrNo == 'y'){
						changedVarNames.insert(variableName);
						flag = 1;
						llvm::errs() << "Rewritten. \n";
						variableName[i]-=32;
					}
					else{
						flag-=1 ;
					}
				}
				else if(variableName[i] >= 'a' && variableName[i]<= 'z' && flag == 1){
					variableName[i]-= 32;
				}
			}
			if(yesOrNo == 'y'){
				_rewriter->ReplaceText(varDecl->getLocation(), variableName.length(), variableName);
			}
		}

		else if(variableName[0]>='A' && variableName[0]<= 'Z'){
			char yesOrNo;
			std::string changeName = variableName;
			changeName[0]+=32;
			llvm::errs() << "WARNING-Variable: " <<variableName << " starts uppercase. \n";
			llvm::errs() << "Consider Changing to: " << changeName << "\n";
			llvm::errs() << "Make Change?(y/n): ";
			std::cin >> yesOrNo;
			if(yesOrNo == 'y'){
				changedVarNames.insert(variableName);
				_rewriter->ReplaceText(varDecl->getLocation(),variableName.length(), changeName);
				llvm::errs() << "Rewritten \n";				
			}
		}
	  }
	  return true;
  }
  
  bool VisitDeclRefExpr(DeclRefExpr *declRefExpr){
	if(declRefExpr){
		ValueDecl *valueDecl = declRefExpr->getDecl();
		QualType qualType = valueDecl->getType();
		std::string variableName = valueDecl->getNameAsString();
		if(changedVarNames.find(variableName) != changedVarNames.end()){
			if(qualType.isConstQualified()){
				for(int i = 0; i<int(variableName.length());i++){
					if(variableName[i]>='a' && variableName[i] <='z'){
						variableName[i]-=32;	//turn to uppercase
					}
				}
			}
			else if(variableName[0] >= 'A' && variableName[0] <= 'Z'){
				variableName[0] = variableName[0] +'a' -'A';
			}
			_rewriter->ReplaceText(declRefExpr->getLocation(), variableName.length(), variableName);
		}
	}
	return true;
  }

  bool VisitFunctionDecl(FunctionDecl *functionDecl){
	  if(!functionDecl)
		  return true;

	  std::string functionName = functionDecl->getNameAsString();

	  if(functionName.length()<2){
		  llvm::errs()<<"WARNING-Function: " << functionName << "is under 2 characters. Lengthen. \n";
	  }

	  if(functionName.length()>15){
		  llvm::errs()<< "WARNING-Function: " <<functionName << " over 15 characters. Shorten.\n";
	  }

	  for(int i =0; i<int(functionName.length()); i++){
		  char inspect = functionName[i];
		  if(inspect =='1'||inspect=='0'||inspect=='l'||inspect=='O'){
			  llvm::errs() << "WARNING-Function: " <<functionName << " contains " << inspect;
			  llvm::errs() << ". Consider Revising.\n";
		  }
	  }

	  if(functionName[0] >='A' && functionName[0]<='Z'){
		  char yesOrNo='N';
		  llvm::errs() << "WARNING-Function: " << functionName << " starts with capital letter.\n";
		  std:: string changeName = functionName;
		  changeName[0]+=32;
		  llvm::errs() << "Replace with: " <<changeName<<"?(y/n):";
		  std::cin >> yesOrNo;

		  if(yesOrNo == 'y'){
			  llvm::errs() << "Rewritten.\n";
			  changedVarNames.insert(functionName);
			  _rewriter->ReplaceText(functionDecl->getLocation(), functionName.length(), changeName);
		  }
	  }

	  return true;
  }

  bool TraverseForStmt(ForStmt *forStmt){
	if(!forStmt)
		return true;
	for_flag = 1;

	Stmt *init = forStmt->getInit();
	if(!init)
		return true;
	RecursiveASTVisitor<StyleVisitor>::TraverseStmt(init);

	Expr *cond = forStmt->getCond();
	if(!cond)
		return true;

	Expr *inc = forStmt->getInc();
	if(!inc)
		return true;

	for_flag = 0;

	Stmt *body = forStmt->getBody();
	if(!body)
		return true;
	RecursiveASTVisitor<StyleVisitor>::TraverseStmt(body);

	return true;
  }
 	 
  bool VisitCallExpr(CallExpr *callExpr){
	FunctionDecl *function = callExpr->getDirectCallee();
	
	if(function){
		if(function->getNameAsString().compare("strcpy")==0){
			Expr *dest = callExpr->getArg(0);
			Expr *src  = callExpr->getArg(1);
			//dest->dump();
			//src->dump();
			dest = dest->IgnoreCasts();
			src = src->IgnoreCasts();

			DeclRefExpr *destRef = dyn_cast<DeclRefExpr>(dest);
			DeclRefExpr *srcRef  = dyn_cast<DeclRefExpr>(src);

			if(destRef != nullptr && srcRef != nullptr){
				destRef->dump();
				srcRef->dump();
				ValueDecl *value = srcRef->getDecl();
				QualType destType = destRef->getDecl()->getType();

				const ConstantArrayType *cat = dyn_cast<ConstantArrayType>(destType);
				
				if(cat){
					std::stringstream ss;
					ss<< value->getNameAsString() << "[" << (cat->getSize().getLimitedValue()-1) << "] = 0;\n\t";
					
					_rewriter->InsertTextBefore(callExpr->getBeginLoc(), ss.str());
				}
			}
		}
	}
	return true;
  }
};

class StyleConsumer : public clang::ASTConsumer{
  StyleVisitor _visitor;

public:
  explicit StyleConsumer(ASTContext *context) : _visitor(context) {}
  
  virtual void HandleTranslationUnit(clang::ASTContext& Context){
    _visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class StyleChecker : public clang::ASTFrontendAction{
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler,
								llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(new StyleConsumer(&Compiler.getASTContext()));
  }
};

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<StyleChecker>().get());
}
