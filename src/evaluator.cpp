struct Evaluator: public DiagnosticHelper {
	bool verbose = true;
    bool error = false;
    Location loc;
    Evaluator(xcc_context &context): DiagnosticHelper{context} {}
	uintmax_t evali(Expr e);
	uintmax_t casti(Expr e);
    uintmax_t withVerbose(Expr e) {
        verbose = true;
        return evali(e);
    }
    uintmax_t withQuiet(Expr e) {
        verbose = false;
        return evali(e);
    }
    uintmax_t withLoc(Expr e, Location theLoc) {
        loc = theLoc;
        verbose = true;
        return evali(e);
    }
    uintmax_t eval_error() {
        error = true;
        return 0;
    }
    void clear_error() {
        error = false;
    }
};

uintmax_t Evaluator::casti(Expr e) {
	auto tags = e->ty->tags;
	auto val = e->castval;
    if (tags & TYBOOL)
        return evali(val) & 1;
    if (tags & (TYINT8 | TYUINT8))
        return (uint8_t)(evali(val));
    if (tags & (TYINT16 | TYUINT16))
        return (uint16_t)evali(val);
    if (tags & (TYINT32 | TYUINT32))
        return (uint32_t)evali(val);
    return evali(val);
}

uintmax_t Evaluator::evali(Expr e) {
    switch (e->k){
    case EBin:
        switch (e->bop) {
        case SAdd:case UAdd:
            return evali(e->lhs) + evali(e->rhs);
        case SSub:case USub:
            return evali(e->lhs) - evali(e->rhs);
        case SMul: case UMul:
            return evali(e->lhs) * evali(e->rhs);
        case UDiv:
            return evali(e->lhs) / evali(e->rhs);
        case SDiv:
            return (intmax_t)evali(e->lhs) / (intmax_t)evali(e->rhs);
        case URem:
            return evali(e->lhs) % evali(e->rhs);
        case SRem:
            return (intmax_t)evali(e->lhs) % (intmax_t)evali(e->rhs);
        case Shr:
            return evali(e->lhs) >> evali(e->rhs);
        case AShr:
            return (intmax_t)evali(e->lhs) >> evali(e->rhs);
        case EQ:
            return evali(e->lhs) == evali(e->rhs);
        case NE:
            return evali(e->lhs) != evali(e->rhs);
        case UGE:
            return evali(e->lhs) >= evali(e->rhs);
        case UGT:
            return evali(e->lhs) > evali(e->rhs);
        case ULE:
            return evali(e->lhs) <= evali(e->rhs);
        case ULT:
            return evali(e->lhs) < evali(e->rhs);
        case SGE:
            return (intmax_t)evali(e->lhs) >= (intmax_t)evali(e->rhs);
        case SGT:
            return (intmax_t)evali(e->lhs) > (intmax_t)evali(e->rhs);
        case SLE:
            return (intmax_t)evali(e->lhs) <= (intmax_t)evali(e->rhs);
        case SLT:
            return (intmax_t)evali(e->lhs) < (intmax_t)evali(e->rhs);
        case And:
            return evali(e->lhs) & evali(e->rhs);
        case Xor:
            return evali(e->lhs) ^ evali(e->rhs);
        case Or:
            return evali(e->lhs) | evali(e->rhs);
        case LogicalAnd:
            return evali(e->lhs) && evali(e->rhs);
        case LogicalOr:
            return evali(e->lhs) || evali(e->rhs);
        default:
            if (verbose) 
                pp_error(loc, "cannot eval binary operator %s", show(e->bop));
            return eval_error();
    }
    case EUnary:
        switch(e->uop) {
        case UNeg:
            return -evali(e->uoperand);
        case SNeg:
            return -(intmax_t(evali(e->uoperand)));
        case LogicalNot:
            return !evali(e->uoperand);
        case Not:
            return ~evali(e->uoperand);
        default:
            if (verbose)
                pp_error(loc, "bad unary operator %s", show(e->uop));
            return eval_error();
        }break;
        case ECast:
            switch(e->castop) {
                case ZExt:case SExt:
                    return evali(e->castval);
                case Trunc:
                    return casti(e);
                default:
                    if (verbose)
                        pp_error("cannot cast to %T", e->ty);
                    return eval_error();
            }
        case EIntLit:
            return e->ival.getLimitedValue();
        case EFloatLit:
            if (verbose)
                pp_error("floating constant in constant-expression");
            return eval_error();
        case ECondition:
            return evali(e->cond) ? evali(e->cleft) : evali(e->cright);
        default:
            if (verbose) 
                pp_error("cannot eval %E", e);
            return eval_error();
    }
}
