package cse262.slang.Parser;

import java.security.InvalidParameterException;
import java.util.List;

import cse262.slang.Parser.INodeVisitor.INodeVisitorError;

/**
 * Nodes is a wrapper class within which we define all of the node types that
 * can appear in an AST. As with Scanner.Tokens, we organize the code in this
 * way so that we do not have to create separate files for the different nodes.
 *
 * Any class defined in this file needs to be something that can be produced by
 * the parser.
 */
public class AstNodes {
    /**
     * Node is the base class for all of the nodes in our abstract syntax tree.
     * Even though Node doesn't have any local fields, we define it as an
     * abstract class, so that the AST infrastructure is symmetric with the
     * Token infrastructure.
     */
    public static abstract class AstNode {
        /**
         * `serialize()` implements a visitor pattern, enabling a recursive
         * descent walk through an AST to serialize it to Xml or other output
         * formats.
         *
         * @param visitor The visitor object
         */
        public abstract void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError;
    }

    /**
     * Placing the Datum type into the hierarchy helps us to write a correct
     * parser.
     *
     * We can use Datum to "tag" certain types, indicating that they are data,
     * not executable forms. If we do this correctly, then the parser can't
     * accidentally put an executable form in a place where a value is expected.
     */
    public static abstract class Datum extends AstNode {
    };

    /**
     * Cons is a pair (cons cell)
     */
    public static class Cons extends Datum {
        /** The first value of the pair */
        public Datum car;

        /** The second value of the pair */
        public Datum cdr;

        /**
         * Create a Cons list from a list of values
         * 
         * @param items The list of values to create a Cons list from
         * 
         * @return The Cons list
         */
        public static Datum makeConsList(List<Datum> items) {
            if (items.size() == 0)
                return new EmptyCons();
            if (items.size() == 1)
                return new Cons(items.get(0), new EmptyCons());
            return new Cons(items.get(0), makeConsList(items.subList(1, items.size())));
        }

        /**
         * Create a Cons cell from two values
         * 
         * @param car The first value
         * @param cdr The second value
         * 
         * @return The Cons cell
         */
        public static Datum makeCons(Datum car, Datum cdr) throws InvalidParameterException {
            if (car == null && cdr == null)
                // return new EmptyCons();
                throw new InvalidParameterException("Arguments to makeCons cannot be null");
            return new Cons(car, cdr);
        }

        /**
         * Construct a `cons` node from two values
         * 
         * @param car The first value
         * @param cdr The second value
         */
        private Cons(Datum car, Datum cdr) {
            this.car = car;
            this.cdr = cdr;
        }


        /**
         * Serialize the Cons cell
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitCons(car, cdr, this);
        }
    }

    /** Vec represents a Vector value */
    public static class Vec extends Datum {
        /** The values represented by this node */
        public final Datum[] items;

        /**
         * Construct a `vec` node from a list of values
         * 
         * @param items The list of values to create a vector from
         */
        public Vec(List<Datum> items) {
            Datum[] tmp = new Datum[items.size()];
            this.items = items.toArray(tmp);
        }


        /**
         * Serialize the vector
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitVec(items, this);
        }
    }

    /**
     * BoolTrue is used to represent the value `true`. We use the *type* to
     * indicate true, instead of some field of the object.
     */
    public static class BoolTrue extends Datum {
        /** Construct a `BoolTrue` node */
        public BoolTrue() {
        }


        /**
         * Serialize the boolean true value
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitBoolTrue(this);
        }
    }

    /**
     * BoolFalse is used to represent the value `false`. We use the *type* to
     * indicate false, instead of some field of the object.
     */
    public static class BoolFalse extends Datum {
        /** Construct a `BoolFalse` node */
        public BoolFalse() {
        }


        /**
         * Serialize the boolean false value
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitBoolFalse(this);
        }
    }

    /** Char is used to represent character values */
    public static class Char extends Datum {
        /** The value represented by this node */
        private final char val;

        /**
         * Construct a `char` node from a character value
         * 
         * @param val The character value
         */
        public Char(char val) {
            this.val = val;
        }


        /**
         * Serialize the character value
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitChar(this.val, this);
        }
    }

    /** EmptyCons is used to represent an empty Cons cell */
    public static class EmptyCons extends Datum {
        /** Construct an `EmptyCons` */
        public EmptyCons() {
        }


        /**
         * Serialize the empty Cons cell
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitEmptyCons(this);
        }
    }

    /** Dbl is used to represent double values */
    public static class Dbl extends Datum {
        /** The value represented by this node */
        public final double val;

        /**
         * Construct a `dbl` node from a double value
         * 
         * @param val The double value
         */
        public Dbl(double val) {
            this.val = val;
        }


        /**
         * Serialize the double value
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitDbl(val, this);
        }
    }

    /** Int is used to represent integer values */
    public static class Int extends Datum {
        /** The value represented by this node */
        public final int val;

        /**
         * Construct an `int` node from an integer value
         * 
         * @param val The integer value
         */
        public Int(int val) {
            this.val = val;
        }


        /**
         * Serialize the integer value
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitInt(val, this);
        }
    }

    /** Str is used to represent String values */
    public static class Str extends Datum {
        /** The value represented by this node */
        public final String val;

        /**
         * Construct a `str` node from a String value
         * 
         * @param val The String value
         */
        public Str(String val) {
            this.val = val;
        }


        /**
         * Serialize the String value
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitStr(val, this);
        }
    }

    /**
     * Symbol represents a symbol value
     *
     * NB: Since Scheme is homo-iconic, Symbol and Identifier look identical.
     * However, a Symbol *is* a value, and an Identifier *is not*
     */
    public static class Symbol extends Datum {
        /** The name associated with this symbol */
        private final String name;

        /**
         * Construct a `symbol` node from its name
         * 
         * @param name The name associated with this symbol
         */
        public Symbol(String name) {
            this.name = name;
        }


        /**
         * Serialize the symbol value
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitSymbol(name, this);
        }
    }

    /** The `and` special form */
    public static class And extends AstNode {
        /** The expressions to evaluate when computing `and` */
        private final List<AstNode> expressions;

        /**
         * Construct an `and` node from a list of expressions
         * 
         * @param expressions The expressions to evaluate when computing `and`
         */
        public And(List<AstNode> expressions) {
            this.expressions = expressions;
        }


        /**
         * Serialize the `and` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitAnd(expressions);
        }
    }

    /** The function call form (the default form) */
    public static class Call extends AstNode {
        /** The set of expressions that comprise this call */
        private final List<AstNode> expressions;

        /**
         * Construct a call node from a list of expressions
         * 
         * @param expressions The expressions to evaluate when computing `call`
         */
        public Call(List<AstNode> expressions) {
            this.expressions = expressions;
        }


        /**
         * Serialize the call expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitCall(expressions);
        }
    }

    /** The `begin` special form */
    public static class Begin extends AstNode {
        /** The expressions to evaluate when computing `begin` */
        private final List<AstNode> expressions;

        /**
         * Construct a `begin` node from a list of expressions
         * 
         * @param expressions The expressions to evaluate when computing `begin`
         */
        public Begin(List<AstNode> expressions) {
            this.expressions = expressions;
        }


        /**
         * Serialize the `begin` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitBegin(expressions);
        }
    }

    /** The `cond` special form */
    public static class Cond extends AstNode {
        /** A condition within a Cond */
        public static class Condition {
            /** The test */
            public final AstNode test;

            /** The expressions to evaluate if the condition is true */
            public final List<AstNode> exprs;

            /**
             * Construct a condition from a test and set of expressions
             * 
             * @param test  The test to evaluate
             * @param exprs The expressions to evaluate if the condition is true
             */
            public Condition(AstNode test, List<AstNode> exprs) {
                this.test = test;
                this.exprs = exprs;
            }
        }

        /** The set of conditions for this Cond */
        private List<Condition> conditions;

        /**
         * Construct a `cond` from a set of expressions
         * 
         * @param conditions The set of conditions for this Cond
         */
        public Cond(List<Condition> conditions) {
            this.conditions = conditions;
        }


        /**
         * Serialize the `cond` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitCond(conditions);
        }
    }

    /** The `define (variable)` special form */
    public static class DefineVar extends AstNode {
        /** The identifier whose value is being defined */
        private final Identifier identifier;

        /** The expression to evaluate when deciding the value to define */
        private final AstNode expression;

        /**
         * Construct a `define` node from an identifier and expression
         * 
         * @param identifier The identifier whose value is being defined
         * @param expression The expression to evaluate when deciding the value to
         *                   define
         */
        public DefineVar(Identifier identifier, AstNode expression) {
            this.identifier = identifier;
            this.expression = expression;
        }


        /**
         * Serialize the `define` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitDefineVar(identifier, expression);
        }
    }

    /** The `define (function)` special form */
    public static class DefineFunc extends AstNode {
        /** The identifier whose value is being defined */
        private final List<Identifier> ids;

        /** The expression to evaluate when deciding the value to define */
        private final List<AstNode> body;

        /**
         * Construct a `define` node from an identifier and expression
         * 
         * @param ids  The identifiers whose values are being defined
         * @param body The expressions to evaluate when deciding the values to define
         */
        public DefineFunc(List<Identifier> ids, List<AstNode> body) {
            this.ids = ids;
            this.body = body;
        }


        /**
         * Serialize the `define` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitDefineFunc(ids, body);
        }
    }

    /** Identifier represents a name (identifier) that can be bound to a value */
    public static class Identifier extends AstNode {
        /** The name associated with this identifier */
        public final String name;

        /** Construct an `identifier` node from its name */
        public Identifier(String name) {
            this.name = name;
        }


        /**
         * Serialize the `identifier` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitIdentifier(name);
        }
    }

    /** The `if` special form */
    public static class If extends AstNode {
        /** The expression to evaluate to true or false */
        private final AstNode test;

        /** The expression to evaluate if true */
        private final AstNode ifTrue;

        /** The expression to evaluate if false */
        private final AstNode ifFalse;

        /**
         * Construct an `if` node from its test, true, and false expressions
         * 
         * @param test    The test to evaluate
         * @param ifTrue  The expression to evaluate if true
         * @param ifFalse The expression to evaluate if false
         */
        public If(AstNode test, AstNode ifTrue, AstNode ifFalse) {
            this.test = test;
            this.ifTrue = ifTrue;
            this.ifFalse = ifFalse;
        }


        /**
         * Serialize the `if` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitIf(test, ifTrue, ifFalse);
        }
    }

    /** The `lambda` special form */
    public static class LambdaDef extends AstNode {
        /** The identifiers that are the formal arguments to the function */
        public final List<Identifier> formals;

        /** The body of the function */
        public final List<AstNode> exprs;

        /**
         * Construct a `lambda` node from its formals and body
         * 
         * @param formals The identifiers that are the formal arguments to the function
         * @param exprs   The body of the function
         */
        public LambdaDef(List<Identifier> formals, List<AstNode> exprs) {
            this.formals = formals;
            this.exprs = exprs;
        }


        /**
         * Serialize the `lambda` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitLambdaDef(formals, exprs, this);
        }
    }

    /** The `or` special form */
    public static class Or extends AstNode {
        /** The expressions to evaluate when computing `or` */
        private final List<AstNode> expressions;

        /**
         * Construct an `or` node from a list of expressions
         * 
         * @param expressions The expressions to evaluate when computing `or`
         */
        public Or(List<AstNode> expressions) {
            this.expressions = expressions;
        }


        /**
         * Serialize the `or` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitOr(expressions);
        }
    }

    /** The `quote` special form' */
    public static class Quote extends AstNode {
        /** The value that is being quoted */
        private final Datum datum;

        /**
         * Construct a `quote` node from the datum being quoted
         * 
         * @param datum The datum being quoted
         */
        public Quote(Datum datum) {
            this.datum = datum;
        }


        /**
         * Serialize the `quote` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitQuote(datum);
        }
    }

    /** The `set!` special form */
    public static class Set extends AstNode {
        /** The identifier whose value is being set */
        private final Identifier id;

        /** The expression to evaluate when deciding the value to set! */
        private final AstNode expr;

        /**
         * Construct a `set!` node from an identifier and expression
         * 
         * @param id   The identifier whose value is being set
         * @param expr The expression to evaluate when deciding the value to set!
         */
        public Set(Identifier id, AstNode expr) {
            this.id = id;
            this.expr = expr;
        }


        /**
         * Serialize the `set!` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitSet(id, expr);
        }
    }

    /** The `'` special form */
    public static class Tick extends AstNode {
        /** The value that is being quoted */
        private final Datum datum;

        /**
         * Construct a `'` node from the datum being quoted
         * 
         * @param datum The datum being quoted
         */
        public Tick(Datum datum) {
            this.datum = datum;
        }


        /**
         * Serialize the `'` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitTick(datum);
        }
    }

    /** The `let` special form */
    public static class Let extends AstNode {
        /**
         * `LetDef` is a child of `let`, useful for describing a single variable
         * binding
         */
        public static class LetDef {
            /** The name of the variable being bound */
            public final Identifier id;

            /**
             * The expression that will produce an initial value for the
             * variable
             */
            public final AstNode val;

            /**
             * Construct a `LetDef` node from an identifier and an expression
             * 
             * @param id  The name of the variable being bound
             * @param val The expression that will produce an initial value for the
             *            variable
             */
            public LetDef(Identifier id, AstNode val) {
                this.id = id;
                this.val = val;
            }
        }

        /** The bindings that are active during the execution of the body */
        private final List<LetDef> vars;

        /** The expressions to evaluate while the variables are bound */
        private final List<AstNode> body;

        /**
         * Construct a `Let` node from a list of bindings and a list of
         * expressions
         * 
         * @param vars The bindings that are active during the execution of the body
         * @param body The expressions to evaluate while the variables are bound
         */
        public Let(List<LetDef> vars, List<AstNode> body) {
            this.vars = vars;
            this.body = body;
        }


        /**
         * Serialize the `let` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitLet(vars, body);
        }
    }

    /** The `apply` special form */
    public static class Apply extends AstNode {
        /** An expression that produces the function to call */
        private AstNode func;

        /** The arguments to the function... Should be a Cons cell */
        private AstNode args;

        /**
         * Construct an `Apply` node from an expression that should produce a
         * function, and a list of expressions that should produce arguments
         * 
         * @param func The function to apply
         * @param args The arguments to apply to the function
         */
        public Apply(AstNode func, AstNode args) {
            this.func = func;
            this.args = args;
        }


        /**
         * Serialize the `apply` expression
         * 
         * @param visitor The visitor object
         */
        @Override
        public void serialize(INodeVisitor<Void> visitor) throws INodeVisitorError {
            visitor.visitApply(func, args);
        }
    }
}
