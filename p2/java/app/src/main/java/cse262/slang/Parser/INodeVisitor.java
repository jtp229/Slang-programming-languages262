package cse262.slang.Parser;

import java.util.List;

/**
 * INodeVisitor is a visitor over all of the AST Node types. It's a small bit of
 * overkill for us to use a Visitor pattern here, since we could just make all
 * fields of all nodes into public final fields, and then access them directly.
 * However, the logic behind the pattern is really nice, especially for
 * interpretation, so we'll go with it.
 *
 * NB: There's not much point in commenting the methods of this interface. They
 * correspond to the different Node types. The key point is that, from a
 * software maintenance perspective, if we were to add new Node types, the use
 * of the Visitor pattern would result in the compiler telling us everywhere
 * that we would need to make edits in order to get those new node types to
 * work.
 */
public interface INodeVisitor<T> {
    public static class INodeVisitorError extends Exception {
        public INodeVisitorError(String message) {
            super(message);
        }
    }

    public T visitAnd(List<AstNodes.AstNode> expressions) throws INodeVisitorError;

    public T visitCall(List<AstNodes.AstNode> expressions) throws INodeVisitorError;

    public T visitBegin(List<AstNodes.AstNode> expressions) throws INodeVisitorError;

    public T visitBoolTrue(AstNodes.BoolTrue datum) throws INodeVisitorError;

    public T visitBoolFalse(AstNodes.BoolFalse datum) throws INodeVisitorError;

    public T visitChar(char val, AstNodes.Char datum) throws INodeVisitorError;

    public T visitCons(AstNodes.Datum car, AstNodes.Datum cdr, AstNodes.Cons datum) throws INodeVisitorError;

    public T visitEmptyCons(AstNodes.EmptyCons datum) throws INodeVisitorError;

    public T visitDefineVar(AstNodes.Identifier identifier, AstNodes.AstNode expression) throws INodeVisitorError;

    public T visitDefineFunc(List<AstNodes.Identifier> ids, List<AstNodes.AstNode> body) throws INodeVisitorError;

    public T visitIdentifier(String name) throws INodeVisitorError;

    public T visitIf(AstNodes.AstNode test, AstNodes.AstNode ifTrue, AstNodes.AstNode ifFalse) throws INodeVisitorError;

    public T visitLambdaDef(List<AstNodes.Identifier> formals, List<AstNodes.AstNode> body,
            AstNodes.LambdaDef lambdaDef) throws INodeVisitorError;

    public T visitInt(int val, AstNodes.Int datum) throws INodeVisitorError;

    public T visitDbl(double val, AstNodes.Dbl datum) throws INodeVisitorError;

    public T visitOr(List<AstNodes.AstNode> expressions) throws INodeVisitorError;

    public T visitQuote(AstNodes.Datum datum) throws INodeVisitorError;

    public T visitSet(AstNodes.Identifier identifier, AstNodes.AstNode expression) throws INodeVisitorError;

    public T visitStr(String value, AstNodes.Str datum) throws INodeVisitorError;

    public T visitSymbol(String name, AstNodes.Symbol datum) throws INodeVisitorError;

    public T visitTick(AstNodes.Datum datum) throws INodeVisitorError;

    public T visitVec(AstNodes.Datum[] items, AstNodes.Vec datum) throws INodeVisitorError;

    public T visitCond(List<AstNodes.Cond.Condition> conditions) throws INodeVisitorError;

    public T visitLet(List<AstNodes.Let.LetDef> vars, List<AstNodes.AstNode> body) throws INodeVisitorError;

    public T visitApply(AstNodes.AstNode func, AstNodes.AstNode args) throws INodeVisitorError;
}