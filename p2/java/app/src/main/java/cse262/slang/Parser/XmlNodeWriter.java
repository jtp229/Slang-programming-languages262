package cse262.slang.Parser;

import java.io.OutputStream;
import java.util.List;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

import cse262.slang.Scanner.XmlHelpers;

public class XmlNodeWriter implements INodeVisitor<Void> {
    /** The current element, that we can add nodes to */
    private Element current;

    /** The document, in which we can create new elements */
    private Document doc;

    /**
     * Visit each node in a forest of Ast nodes, and serialize them all to the
     * given stream as XML
     * 
     * @param forest The list of Ast nodes to serialize
     * @param stream The stream to serialize to
     * @throws INodeVisitorError If the forest is not valid
     */
    public void astToXml(List<AstNodes.AstNode> forest, OutputStream stream) throws INodeVisitorError {
        // Create an XML document
        try {
            var builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            doc = builder.newDocument();
        } catch (ParserConfigurationException e) {
            System.err.println("Unexpected error constructing an XML DocumentBuilder.");
            e.printStackTrace();
            return;
        }
        // Now make the root element
        current = doc.createElement("Ast");
        current.setAttribute("xmlns", "ast");
        doc.appendChild(current);

        for (var n : forest)
            n.serialize(this);

        // Write the Xml document to the provided stream
        try {
            // Configure the transformer, which actually does the writing. We
            // want to skip a leading `<xml>` tag, and we want indentation
            var transformer = TransformerFactory.newInstance().newTransformer();
            transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
            transformer.setOutputProperty(OutputKeys.INDENT, "yes");
            transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "1");

            // Now go ahead and write the document to the stream
            transformer.transform(new DOMSource(doc), new StreamResult(stream));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Serialize an and node
     * 
     * @param expressions The list of expressions
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitAnd(List<AstNodes.AstNode> expressions) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_AND);
        old.appendChild(current);
        for (var e : expressions)
            e.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a call node
     * 
     * @param expressions The list of expressions
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitCall(List<AstNodes.AstNode> expressions) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_CALL);
        old.appendChild(current);
        for (var e : expressions)
            e.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a begin node
     * 
     * @param expressions The list of expressions
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitBegin(List<AstNodes.AstNode> expressions) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_BEGIN);
        old.appendChild(current);
        for (var e : expressions)
            e.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a boolean true node
     * 
     * @param datum The boolean true node
     */
    @Override
    public Void visitBoolTrue(AstNodes.BoolTrue datum) {
        var node = doc.createElement(XmlConstants.TAG_BOOL);
        node.setAttribute("val", "true");
        current.appendChild(node);
        return null;
    }

    /**
     * Serialize a boolean false node
     * 
     * @param datum The boolean false node
     */
    @Override
    public Void visitBoolFalse(AstNodes.BoolFalse datum) {
        var node = doc.createElement(XmlConstants.TAG_BOOL);
        node.setAttribute("val", "false");
        current.appendChild(node);
        return null;
    }

    /**
     * Serialize a character node
     * 
     * @param val   The character value
     * @param datum The character node
     */
    @Override
    public Void visitChar(char val, AstNodes.Char datum) {
        var node = doc.createElement(XmlConstants.TAG_CHAR);
        node.setAttribute("val", XmlHelpers.escape("" + val));
        current.appendChild(node);
        return null;
    }

    /**
     * Serialize a cons node
     * 
     * @param car   The car of the cons
     * @param cdr   The cdr of the cons
     * @param datum The cons node
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitCons(AstNodes.Datum car, AstNodes.Datum cdr, AstNodes.Cons datum) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_CONS);
        old.appendChild(current);
        car.serialize(this);
        cdr.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize an empty cons node
     * 
     * @param datum The empty cons node
     */
    @Override
    public Void visitEmptyCons(AstNodes.EmptyCons datum) {
        current.appendChild(doc.createElement(XmlConstants.TAG_EMPTY));
        return null;
    }

    /**
     * Serialize a define variable node
     * 
     * @param identifier The identifier to define
     * @param expression The expression to define
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitDefineVar(AstNodes.Identifier identifier, AstNodes.AstNode expression) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_DEFINE_VAR);
        old.appendChild(current);
        identifier.serialize(this);
        expression.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a define function node
     * 
     * @param ids  The list of identifiers
     * @param body The list of expressions
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitDefineFunc(List<AstNodes.Identifier> ids, List<AstNodes.AstNode> body) throws INodeVisitorError {
        var old = current;
        var I = doc.createElement(XmlConstants.TAG_IDENTIFIERS);
        var E = doc.createElement(XmlConstants.TAG_EXPRESSIONS);
        var D = doc.createElement(XmlConstants.TAG_DEFINE_FUNC);
        D.appendChild(I);
        D.appendChild(E);
        old.appendChild(D);
        current = I;
        for (var i : ids)
            i.serialize(this);
        current = E;
        for (var e : body)
            e.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize an identifier node
     * 
     * @param name The name of the identifier
     */
    @Override
    public Void visitIdentifier(String name) {
        var node = doc.createElement(XmlConstants.TAG_IDENTIFIER);
        node.setAttribute("val", name);
        current.appendChild(node);
        return null;
    }

    /**
     * Serialize an if node
     * 
     * @param test    The test expression
     * @param ifTrue  The expression to evaluate if the test is true
     * @param ifFalse The expression to evaluate if the test is false
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitIf(AstNodes.AstNode test, AstNodes.AstNode ifTrue, AstNodes.AstNode ifFalse)
            throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_IF);
        old.appendChild(current);
        test.serialize(this);
        ifTrue.serialize(this);
        ifFalse.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a lambda definition node
     * 
     * @param formals The list of formal parameters
     * @param body    The list of expressions in the body
     * @param node    The lambda definition node
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitLambdaDef(List<AstNodes.Identifier> formals, List<AstNodes.AstNode> body,
            AstNodes.LambdaDef node) throws INodeVisitorError {
        var old = current;
        var L = doc.createElement(XmlConstants.TAG_LAMBDA);
        var F = doc.createElement(XmlConstants.TAG_FORMALS);
        var E = doc.createElement(XmlConstants.TAG_EXPRESSIONS);
        L.appendChild(F);
        L.appendChild(E);
        old.appendChild(L);
        current = F;
        for (var f : formals)
            f.serialize(this);
        current = E;
        for (var b : body)
            b.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize an integer node
     * 
     * @param val   The integer value
     * @param datum The integer node
     */
    @Override
    public Void visitInt(int val, AstNodes.Int datum) {
        var node = doc.createElement(XmlConstants.TAG_INT);
        node.setAttribute("val", "" + val);
        current.appendChild(node);
        return null;
    }

    /**
     * Serialize a double node
     * 
     * @param val   The double value
     * @param datum The double node
     */
    @Override
    public Void visitDbl(double val, AstNodes.Dbl datum) {
        var node = doc.createElement(XmlConstants.TAG_DBL);
        node.setAttribute("val", XmlHelpers.formatDouble(val));
        current.appendChild(node);
        return null;
    }

    /**
     * Serialize an or node
     * 
     * @param expressions The list of expressions
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitOr(List<AstNodes.AstNode> expressions) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_OR);
        old.appendChild(current);
        for (var e : expressions)
            e.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a quote node
     * 
     * @param datum The quoted datum
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitQuote(AstNodes.Datum datum) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_QUOTE);
        old.appendChild(current);
        datum.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a set variable node
     * 
     * @param identifier The identifier to set
     * @param expression The expression to set
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitSet(AstNodes.Identifier identifier, AstNodes.AstNode expression) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_SET);
        old.appendChild(current);
        identifier.serialize(this);
        expression.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a string node
     * 
     * @param value The string value
     * @param datum The string node
     */
    @Override
    public Void visitStr(String value, AstNodes.Str datum) {
        var node = doc.createElement(XmlConstants.TAG_STR);
        node.setAttribute("val", XmlHelpers.escape(value));
        current.appendChild(node);
        return null;
    }

    /**
     * Serialize a symbol node
     * 
     * @param name  The name of the symbol
     * @param datum The symbol node
     */
    @Override
    public Void visitSymbol(String name, AstNodes.Symbol datum) {
        var node = doc.createElement(XmlConstants.TAG_SYMBOL);
        node.setAttribute("val", name);
        current.appendChild(node);
        return null;
    }

    /**
     * Serialize a tick node
     * 
     * @param datum The datum to tick
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitTick(AstNodes.Datum datum) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_TICK);
        old.appendChild(current);
        datum.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a vector node
     * 
     * @param items The items in the vector
     * @param datum The vector node
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitVec(AstNodes.Datum[] items, AstNodes.Vec datum) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_VECTOR);
        old.appendChild(current);
        for (var i : items)
            i.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a cond node
     * 
     * @param conditions The list of conditions
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitCond(List<AstNodes.Cond.Condition> conditions) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_COND);
        old.appendChild(current);
        for (var c : conditions)
            serializeCond(c.test, c.exprs);
        current = old;
        return null;
    }

    /**
     * Serialize a cond node
     * 
     * @param test        The test expression
     * @param expressions The list of expressions
     * @throws INodeVisitorError If the node is not valid
     */
    public void serializeCond(AstNodes.AstNode test, List<AstNodes.AstNode> expressions) throws INodeVisitorError {
        var old = current;
        var C = doc.createElement(XmlConstants.TAG_CONDITION);
        var T = doc.createElement(XmlConstants.TAG_TEST);
        var A = doc.createElement(XmlConstants.TAG_ACTIONS);
        C.appendChild(T);
        C.appendChild(A);
        old.appendChild(C);
        current = T;
        test.serialize(this);
        current = A;
        for (var a : expressions)
            a.serialize(this);
        current = old;
    }

    /**
     * Serialize a let node
     * 
     * @param vars The list of let bindings
     * @param body The list of expressions in the body
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitLet(List<AstNodes.Let.LetDef> vars, List<AstNodes.AstNode> body) throws INodeVisitorError {
        var old = current;
        var L = doc.createElement(XmlConstants.TAG_LET);
        var B = doc.createElement(XmlConstants.TAG_BINDINGS);
        var A = doc.createElement(XmlConstants.TAG_ACTIONS);
        L.appendChild(B);
        L.appendChild(A);
        old.appendChild(L);
        current = B;
        for (var v : vars)
            this.serializeLetDef(v.id, v.val);
        current = A;
        for (var e : body)
            e.serialize(this);
        current = old;
        return null;
    }

    /**
     * Serialize a let binding node
     * 
     * @param id  The identifier to bind
     * @param val The value to bind
     * @throws INodeVisitorError If the node is not valid
     */
    public void serializeLetDef(AstNodes.Identifier id, AstNodes.AstNode val) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_BINDING);
        old.appendChild(current);
        id.serialize(this);
        val.serialize(this);
        current = old;
    }

    /**
     * Serialize an apply node
     * 
     * @param func The function to apply
     * @param args The arguments to apply
     * @throws INodeVisitorError If the node is not valid
     */
    @Override
    public Void visitApply(AstNodes.AstNode func, AstNodes.AstNode args) throws INodeVisitorError {
        var old = current;
        current = doc.createElement(XmlConstants.TAG_APPLY);
        old.appendChild(current);
        func.serialize(this);
        args.serialize(this);
        current = old;
        return null;
    }
}
