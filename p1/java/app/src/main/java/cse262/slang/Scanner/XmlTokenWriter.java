package cse262.slang.Scanner;

import java.io.OutputStream;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

import cse262.slang.Scanner.Tokens.*;

/**
 * XmlTokenWriter serializes a TokenStream to Xml, and writes that Xml to an
 * output stream. Since the TokenStream is really just an array of Tokens, the
 * Xml will consist of a root element (<Tokens>), whose children represent all
 * of the tokens from the scanner, in order of appearance in the file.
 *
 * All Xml elements will have a line number, column number, and a representation
 * of their token text.
 *
 * For tokens that have a literal (bool, char, double, int, string), that
 * literal will be an attribute of the Xml element called `val`.
 *
 * Please keep in mind that the primary purpose of this XML-based format is to
 * make it easier for students to get full marks on later phases of the
 * assignment without completing earlier phases. Compilers and interpreters
 * don't usually print their tokens directly, except during debugging.
 */
public class XmlTokenWriter {
    /**
     * Produce an Xml representation of the given TokenStream, and write it to
     * the given stream.
     *
     * @param tokens The TokenStream to write as Xml
     * @param stream Where to write the Xml
     */
    public void writeXmlToStream(TokenStream tokens, OutputStream stream) {
        // Create an XML document in memory. This should always work, but we
        // need a try/catch block anyway...
        var dbf = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder = null;
        try {
            builder = dbf.newDocumentBuilder();
        } catch (ParserConfigurationException e) {
            System.err.println("Unexpected error constructing an XML DocumentBuilder.");
            e.printStackTrace();
            return;
        }
        var doc = builder.newDocument();

        // Now make the root element
        var root = doc.createElement(XmlConstants.tagRoot.value);
        root.setAttribute("xmlns", "tokens");
        doc.appendChild(root);

        // Add all the tokens as children of the root
        while (tokens.hasNext()) {
            var token = tokens.nextToken();
            if (token instanceof Abbrev) {
                tagFromBasicToken(XmlConstants.tagAbbrev, token, doc, root);
            } else if (token instanceof And) {
                tagFromBasicToken(XmlConstants.tagAnd, token, doc, root);
            } else if (token instanceof Apply) {
                tagFromBasicToken(XmlConstants.tagApply, token, doc, root);
            } else if (token instanceof Begin) {
                tagFromBasicToken(XmlConstants.tagBegin, token, doc, root);
            } else if (token instanceof Bool) {
                tagFromValueToken(XmlConstants.tagBool, token, doc, root,
                        ((Bool) token).value ? XmlConstants.valTrue.value : XmlConstants.valFalse.value);
            } else if (token instanceof Char) {
                tagFromValueToken(XmlConstants.tagChar, token, doc, root, XmlHelpers.escape("" + ((Char) token).value));
            } else if (token instanceof Cond) {
                tagFromBasicToken(XmlConstants.tagCond, token, doc, root);
            } else if (token instanceof Dbl) {
                tagFromValueToken(XmlConstants.tagDbl, token, doc, root, ((Dbl) token).value + "");
            } else if (token instanceof Define) {
                tagFromBasicToken(XmlConstants.tagDefine, token, doc, root);
            } else if (token instanceof Dot) {
                tagFromBasicToken(XmlConstants.tagDot, token, doc, root);
            } else if (token instanceof Eof) {
                var elt = doc.createElement(XmlConstants.tagEof.value);
                root.appendChild(elt);
            } else if (token instanceof Identifier) {
                tagFromValueToken(XmlConstants.tagIdentifier, token, doc, root, token.tokenText);
            } else if (token instanceof If) {
                tagFromBasicToken(XmlConstants.tagIf, token, doc, root);
            } else if (token instanceof Int) {
                tagFromValueToken(XmlConstants.tagInt, token, doc, root, "" + ((Int) token).value);
            } else if (token instanceof Lambda) {
                tagFromBasicToken(XmlConstants.tagLambda, token, doc, root);
            } else if (token instanceof LeftParen) {
                tagFromBasicToken(XmlConstants.tagLParen, token, doc, root);
            } else if (token instanceof Let) {
                tagFromBasicToken(XmlConstants.tagLet, token, doc, root);
            } else if (token instanceof Or) {
                tagFromBasicToken(XmlConstants.tagOr, token, doc, root);
            } else if (token instanceof Quote) {
                tagFromBasicToken(XmlConstants.tagQuote, token, doc, root);
            } else if (token instanceof RightParen) {
                tagFromBasicToken(XmlConstants.tagRParen, token, doc, root);
            } else if (token instanceof Set) {
                tagFromBasicToken(XmlConstants.tagSet, token, doc, root);
            } else if (token instanceof Str) {
                tagFromValueToken(XmlConstants.tagStr, token, doc, root, XmlHelpers.escape(((Str) token).value));
            } else if (token instanceof Vec) {
                tagFromBasicToken(XmlConstants.tagVector, token, doc, root);
            } else {
                System.err.println("Unknown token type: " + token);
                System.exit(1);
            }
            tokens.popAny();
        }

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
     * tagFromBasicToken produces an Xml tag for any token that doesn't have a
     * value
     *
     * @param tag  The name to use for the Xml tag
     * @param tok  The token being translated to Xml
     * @param doc  The document into which the tag will go
     * @param root The root element of that document
     */
    private void tagFromBasicToken(XmlConstants.StringConstant tag, Token tok, Document doc, Element root) {
        var elt = doc.createElement(tag.value);
        elt.setAttribute(XmlConstants.attrLine.value, tok.line + "");
        elt.setAttribute(XmlConstants.attrColumn.value, tok.col + "");
        root.appendChild(elt);
    }

    /**
     * tagFromValueToken produces an Xml tag for any token that has a value. The
     * trick is that we pre-format the token as a string before calling this.
     *
     * @param name  The name to use for the Xml tag
     * @param tok   The token being translated to Xml
     * @param doc   The document into which the tag will go
     * @param root  The root element of that document
     * @param value The value of the token, as a String
     */
    private void tagFromValueToken(XmlConstants.StringConstant tag, Token tok, Document doc, Element root,
            String value) {
        var elt = doc.createElement(tag.value);
        elt.setAttribute(XmlConstants.attrLine.value, tok.line + "");
        elt.setAttribute(XmlConstants.attrColumn.value, tok.col + "");
        elt.setAttribute(XmlConstants.attrValue.value, value + "");
        root.appendChild(elt);
    }
}
