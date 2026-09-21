package cse262.slang.Scanner;

import java.io.IOException;
import java.io.StringReader;

import java.util.ArrayList;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * XmlTokenReader is a lightweight parser that can convert a stream of Xml
 * elements that was produced by XmlTokenWriter (end of scan phase) back into a
 * list of tokens that is suitable for the parse phase.
 *
 * Warning: This code assumes that (1) XmlTokenWriter is correct, and (2) the
 * xml given to it was created by XmlTokenWriter. If this code is given invalid
 * Xml, it will not handle errors correctly.
 */
public class XmlTokenReader {
    /**
     * Given a string representation of an Xml file, read Tokens from it and
     * return them as a TokenStream.
     *
     * @param xml A string holding Xml that was presumably produced by
     *            XmlTokenWriter
     *
     * @return A TokenStream of the tokens from `xml`, or null on any error
     */
    public TokenStream readTokensFromXml(String xml) {
        var res = new ArrayList<Tokens.Token>();
        try {
            // Open the Xml document. If it isn't a valid Xml document at all,
            // then this will throw, but if it's just not the right kind of Xml
            // document, we'll crash later.
            var dbf = DocumentBuilderFactory.newInstance();
            dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
            var db = dbf.newDocumentBuilder();
            Document doc = db.parse(new InputSource(new StringReader(xml)));
            doc.getDocumentElement().normalize();

            // Get the children of the root element, and parse them in order
            //
            // NB: We don't even bother to verify that the root tag is an
            // XmlConstants.tagRoot
            var root = doc.getDocumentElement();
            var children = root.getChildNodes();
            for (int i = 0; i < children.getLength(); ++i) {
                var token = children.item(i);
                if (token.getNodeType() == Node.ELEMENT_NODE) {
                    var name = token.getNodeName();
                    var attributes = token.getAttributes();
                    var lineAttr = attributes.getNamedItem(XmlConstants.attrLine.value);
                    var line = lineAttr == null ? 0 : Integer.parseInt(lineAttr.getTextContent());
                    var colAttr = attributes.getNamedItem(XmlConstants.attrColumn.value);
                    var col = colAttr == null ? 0 : Integer.parseInt(colAttr.getTextContent());
                    var valNode = attributes.getNamedItem("val");
                    var val = valNode == null ? "" : valNode.getTextContent();
                    switch (name) {
                        case "AbbrevToken":
                            res.add(new Tokens.Abbrev("'", line, col));
                            break;
                        case "AndToken":
                            res.add(new Tokens.And("and", line, col));
                            break;
                        case "ApplyToken":
                            res.add(new Tokens.Apply("apply", line, col));
                            break;
                        case "BeginToken":
                            res.add(new Tokens.Begin("begin", line, col));
                            break;
                        case "BoolToken":
                            res.add(new Tokens.Bool(val, line, col, "true".equals(val)));
                            break;
                        case "CharToken":
                            res.add(new Tokens.Char(val, line, col, XmlHelpers.unEscape(val).charAt(0)));
                            break;
                        case "CondToken":
                            res.add(new Tokens.Cond("cond", line, col));
                            break;
                        case "DblToken":
                            res.add(new Tokens.Dbl(val, line, col, Double.parseDouble(val)));
                            break;
                        case "DefineToken":
                            res.add(new Tokens.Define("define", line, col));
                            break;
                        case "DotToken":
                            res.add(new Tokens.Dot("dot", line, col));
                            break;
                        case "EofToken":
                            res.add(new Tokens.Eof("", line, col));
                            break;
                        case "IdentifierToken":
                            res.add(new Tokens.Identifier(val, line, col));
                            break;
                        case "IfToken":
                            res.add(new Tokens.If("if", line, col));
                            break;
                        case "IntToken":
                            res.add(new Tokens.Int(val, line, col, Integer.parseInt(val)));
                            break;
                        case "LambdaToken":
                            res.add(new Tokens.Lambda("lambda", line, col));
                            break;
                        case "LeftParenToken":
                            res.add(new Tokens.LeftParen("(", line, col));
                            break;
                        case "LetToken":
                            res.add(new Tokens.Let("let", line, col));
                            break;
                        case "OrToken":
                            res.add(new Tokens.Or("or", line, col));
                            break;
                        case "QuoteToken":
                            res.add(new Tokens.Quote("quote", line, col));
                            break;
                        case "RightParenToken":
                            res.add(new Tokens.RightParen(")", line, col));
                            break;
                        case "SetToken":
                            res.add(new Tokens.Set("set!", line, col));
                            break;
                        case "StrToken":
                            res.add(new Tokens.Str(val, line, col, XmlHelpers.unEscape(val)));
                            break;
                        case "VecToken":
                            res.add(new Tokens.Vec("#{", line, col));
                            break;
                        default:
                            System.err.println("Error parsing Xml file " + name);
                            System.exit(1);
                    }
                }
            }
        } catch (ParserConfigurationException e) {
            e.printStackTrace();
            return null;
        } catch (IOException e) {
            return null;
        } catch (SAXException e) {
            return null;
        }
        // Return a TokenStream from the provided tokens
        //
        // NB: Remember: we didn't validate. For example, we didn't check that
        // there is exactly one EofToken, and that it is the very last
        // token.
        return new TokenStream(res);
    }
}