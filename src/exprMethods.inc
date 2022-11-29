location_t getBeginLoc() const {
    switch (k) {
    case EVar: return varLoc;
    case EBin: return rhs->getBeginLoc();
    case EUnary: return opLoc;
    case ECast: return castval->getBeginLoc();
    case ESubscript: return left->getBeginLoc();
    case EConstant: return constantLoc;
    case ECondition: return cond->getBeginLoc();
    case ECall: return callfunc->getBeginLoc();
    case EConstantArray: return constantArrayLoc;
    case EMemberAccess: return obj->getBeginLoc();
    case EArrToAddress: return arr3->getBeginLoc();
    case EPostFix: return poperand->getBeginLoc();
    case EArray: return ArrayStartLoc;
    case EStruct: return StructStartLoc;
    case EVoid: return voidStartLoc;
    case EConstantArraySubstript: return constantArraySubscriptLoc;
    }
    llvm_unreachable("invalid Expr");
}
location_t getEndLoc() const {
    switch (k) {
    case EConstant:
        return constantEndLoc;
    case EBin:
        return rhs->getSourceRange();
    case EUnary:
        return uoperand->getEndLoc();
    case ESubscript:
        return right->getEndLoc();
    case EConstantArraySubstript:
        return constantArraySubscriptLocEnd;
    case EConstantArray:
        return constantArrayLocEnd;
    case EVoid:
        return voidexpr->getEndLoc();
    case ECast:
        return castval->getEndLoc();
    case EVar:
        return varLoc + varName->getKey()->size() - 1;
    case EMemberAccess:
        return memberEndLoc;
    case EArrToAddress:
        return arr3->getEndLoc();
    case ECondition: 
        return cright->getEndLoc();
    case ECall:
        return callEnd;
    case EPostFix:
        return postFixEndLoc;
    case EArray:
        return ArrayEndLoc;
    case EStruct:
        return StructEndLoc;
    }
    llvm_unreachable("invalid Expr");
}
SourceRange getSourceRange() const {
    return SourceRange(getBeginLoc(), getEndLoc());
}
