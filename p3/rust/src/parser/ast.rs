/// Cons(car: AstNode, cdr: AstNode)
#[derive(Clone)]
pub struct ConsNode {
    pub car: Datum,
    pub cdr: Datum,
}

/// Vector(items: AstNode[])
#[derive(Clone)]
pub struct VecNode {
    pub items: Vec<Datum>,
}

#[derive(Clone)]
pub enum Datum {
    BoolTrue,
    BoolFalse,
    Char(char),
    Cons(Box<ConsNode>),
    EmptyCons,
    Dbl(f64),
    Int(i64),
    Str(String),
    Symbol(String),
    Vec(Box<VecNode>),
}

/// And(expressions: AstNode[])
#[derive(Clone)]
pub struct AndNode {
    pub exprs: Vec<AstNode>,
}

/// Call(expressions: AstNode[])
#[derive(Clone)]
pub struct CallNode {
    pub exprs: Vec<AstNode>,
}

/// Begin(expressions: AstNode[])
#[derive(Clone)]
pub struct BeginNode {
    pub exprs: Vec<AstNode>,
}

/// Condition(test: AstNode, expressions: AstNode[])
#[derive(Clone)]
pub struct ConditionNode {
    pub test: Box<AstNode>,
    pub exprs: Vec<AstNode>,
}

/// Cond(conditions: Condition[])
#[derive(Clone)]
pub struct CondNode {
    pub conditions: Vec<ConditionNode>,
}

/// DefineVar(identifier: Identifier, expression: AstNode)
#[derive(Clone)]
pub struct DefineVarNode {
    pub id: IdentifierNode,
    pub expr: Box<AstNode>,
}

/// DefineFunc(identifiers: Identifier+, expressions: AstNode[])
#[derive(Clone)]
pub struct DefineFuncNode {
    pub ids: Vec<IdentifierNode>,
    pub body: Vec<AstNode>,
}

/// Identifier(id: String)
#[derive(Clone)]
pub struct IdentifierNode {
    pub id: String,
}

// If(cond: AstNode, ifTrue: AstNode, ifFalse: AstNode)
#[derive(Clone)]
pub struct IfNode {
    pub cond: Box<AstNode>,
    pub if_true: Box<AstNode>,
    pub if_false: Box<AstNode>,
}

/// LambdaDef(formals: Identifier[], body: AstNode[])
#[derive(Clone)]
pub struct LambdaDefNode {
    pub formals: Vec<IdentifierNode>,
    pub exprs: Vec<AstNode>,
}

/// Or(expressions: AstNode[])
#[derive(Clone)]
pub struct OrNode {
    pub exprs: Vec<AstNode>,
}

/// Quote(datum: AstNode)
#[derive(Clone)]
pub struct QuoteNode {
    pub datum: Datum,
}

/// Set(identifier: Identifier, expression: AstNode)
#[derive(Clone)]
pub struct SetNode {
    pub id: IdentifierNode,
    pub expr: Box<AstNode>,
}

/// Tick(datum: AstNode)
#[derive(Clone)]
pub struct TickNode {
    pub datum: Datum,
}

/// Let(definitions: LetDef[], body: AstNode[])
#[derive(Clone)]
pub struct LetNode {
    pub vars: Vec<LetDefNode>,
    pub body: Vec<AstNode>,
}

// LetDef(id: identifier, val: AstNode)
#[derive(Clone)]
pub struct LetDefNode {
    pub id: IdentifierNode,
    pub val: Box<AstNode>,
}

/// Apply(func: AstNode, args: AstNode)
#[derive(Clone)]
pub struct ApplyNode {
    pub func: Box<AstNode>,
    pub args: Box<AstNode>,
}

/// AstNodeDetails is the enum of all possible AST Node types
#[derive(Clone)]
pub enum AstNodeDetails {
    And(AndNode),
    Apply(ApplyNode),
    Call(CallNode),
    Begin(BeginNode),
    Condition(ConditionNode),
    Cond(CondNode),
    DefineVar(DefineVarNode),
    DefineFunc(DefineFuncNode),
    Datum(Datum),
    Identifier(IdentifierNode),
    If(IfNode),
    LambdaDef(LambdaDefNode),
    Let(LetNode),
    Or(OrNode),
    Quote(QuoteNode),
    Set(SetNode),
    Tick(TickNode),
}

/// AstNode is the type of all AST Nodes.  It consists of the node details
#[derive(Clone)]
pub struct AstNode {
    /// The node details.  Essentially "what kind of node is this"
    pub details: AstNodeDetails,
}

impl AstNode {
    /// Create an AstNode from the relevant details
    pub fn new(details: AstNodeDetails) -> Self {
        AstNode { details }
    }
}
