#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <cstdio>
#include <memory>
#include <sstream>
#include <string>


using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

Rewriter rewriter;
int numFunctions = 0;


class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
private:
    ASTContext *astContext; // used for getting additional AST info

public:
    explicit ExampleVisitor(CompilerInstance *CI)
    : astContext(&(CI->getASTContext())) // initialize private members
    {
        rewriter.setSourceMgr(astContext->getSourceManager(), astContext->getLangOpts());
    }
    virtual ~ExampleVisitor() {}

    bool VisitStmt(Stmt *s) {
        // Only care about If statements.
        if (isa<IfStmt>(s)) {
            IfStmt *IfStatement = cast<IfStmt>(s);
            Stmt *Then = IfStatement->getThen();

            rewriter.InsertText(Then->getBeginLoc().getLocWithOffset(1), "// the 'if' part", true,
                                true);

            Stmt *Else = IfStatement->getElse();
            if (Else)
                rewriter.InsertText(Else->getEndLoc().getLocWithOffset(1), "// the 'else' part",
                                    true, true);
        }

        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *f) {
        // Only function definitions (with bodies), not declarations.
        if (f->hasBody()) {
            Stmt *FuncBody = f->getBody();

            // Type name as string
            QualType QT = f->getReturnType();
            std::string TypeStr = QT.getAsString();

            // Function name
            DeclarationName DeclName = f->getNameInfo().getName();
            std::string FuncName = DeclName.getAsString();

            // Add comment before
            std::stringstream SSBefore;
            SSBefore << "// Begin function " << FuncName << " returning " << TypeStr
            << "";
            SourceLocation ST = FuncBody->getSourceRange().getBegin().getLocWithOffset(1);
            rewriter.InsertText(ST, SSBefore.str(), true, true);

            // And after
            //            std::stringstream SSAfter;
            //            SSAfter << "// End function " << FuncName;
            //            ST = FuncBody->getEndLoc().getLocWithOffset(1);
            //            rewriter.InsertText(ST, SSAfter.str(), true, true);
        }

        return true;
    }

    /*
     virtual bool VisitReturnStmt(ReturnStmt *ret) {
     rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
     errs() << "** Rewrote ReturnStmt\n";
     return true;
     }

     virtual bool VisitCallExpr(CallExpr *call) {
     rewriter.ReplaceText(call->getLocStart(), 7, "add5");
     errs() << "** Rewrote function call\n";
     return true;
     }
     */
};



class ExampleASTConsumer : public ASTConsumer {
private:
    ExampleVisitor *visitor; // doesn't have to be private

public:
    // override the constructor in order to pass CI
    explicit ExampleASTConsumer(CompilerInstance *CI)
    : visitor(new ExampleVisitor(CI)) // initialize the visitor
    { }

    // override this to call our ExampleVisitor on the entire source file
    virtual void HandleTranslationUnit(ASTContext &Context) {
        /* we can use ASTContext to get the TranslationUnitDecl, which is
         a single Decl that collectively represents the entire source file */
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }

    /*
     // override this to call our ExampleVisitor on each top-level Decl
     virtual bool HandleTopLevelDecl(DeclGroupRef DG) {
     // a DeclGroupRef may have multiple Decls, so we iterate through each one
     for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; i++) {
     Decl *D = *i;
     visitor->TraverseDecl(D); // recursively visit each AST node in Decl "D"
     }
     return true;
     }
     */
};



class ExampleFrontendAction : public ASTFrontendAction {
public:
    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) {
        return (std::unique_ptr<ASTConsumer>)new ExampleASTConsumer(&CI); // pass CI pointer to ASTConsumer
    }
};



int main(int argc, const char **argv) {
    // parse the command-line args passed to your code
    CommonOptionsParser op(argc, argv, *new llvm::cl::OptionCategory("test"));
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // run the Clang Tool, creating a new FrontendAction (explained below)
    int result = Tool.run(newFrontendActionFactory<ExampleFrontendAction>().get());

    //    errs() << "\nFound " << numFunctions << " functions.\n\n";
    // print out the rewritten source code ("rewriter" is a global var.)
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
    return result;
}

