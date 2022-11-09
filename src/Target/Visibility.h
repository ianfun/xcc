 /// Describes the different kinds of linkage
 /// (C++ [basic.link], C99 6.2.2) that an entity may have.
 enum Linkage : unsigned char {
   /// No linkage, which means that the entity is unique and
   /// can only be referred to from within its scope.
   NoLinkage = 0,
  
   /// Internal linkage, which indicates that the entity can
   /// be referred to from within the translation unit (but not other
   /// translation units).
   InternalLinkage,
  
   /// External linkage within a unique namespace.
   ///
   /// From the language perspective, these entities have external
   /// linkage. However, since they reside in an anonymous namespace,
   /// their names are unique to this translation unit, which is
   /// equivalent to having internal linkage from the code-generation
   /// point of view.
   UniqueExternalLinkage,
  
   /// No linkage according to the standard, but is visible from other
   /// translation units because of types defined in a inline function.
   VisibleNoLinkage,
  
   /// Internal linkage according to the Modules TS, but can be referred
   /// to from other translation units indirectly through inline functions and
   /// templates in the module interface.
   ModuleInternalLinkage,
  
   /// Module linkage, which indicates that the entity can be referred
   /// to from other translation units within the same module, and indirectly
   /// from arbitrary other translation units through inline functions and
   /// templates in the module interface.
   ModuleLinkage,
  
   /// External linkage, which indicates that the entity can
   /// be referred to from other translation units.
   ExternalLinkage
 };
  
 /// Describes the different kinds of language linkage
 /// (C++ [dcl.link]) that an entity may have.
 enum LanguageLinkage {
   CLanguageLinkage,
   CXXLanguageLinkage,
   NoLanguageLinkage
 };
  
 /// A more specific kind of linkage than enum Linkage.
 ///
 /// This is relevant to CodeGen and AST file reading.
 enum GVALinkage {
   GVA_Internal,
   GVA_AvailableExternally,
   GVA_DiscardableODR,
   GVA_StrongExternal,
   GVA_StrongODR
 };
  
 inline bool isDiscardableGVALinkage(GVALinkage L) {
   return L <= GVA_DiscardableODR;
 }
  
 /// Do we know that this will be the only definition of this symbol (excluding
 /// inlining-only definitions)?
 inline bool isUniqueGVALinkage(GVALinkage L) {
   return L == GVA_Internal || L == GVA_StrongExternal;
 }
  
 inline bool isExternallyVisible(Linkage L) {
   return L >= VisibleNoLinkage;
 }
  
 inline Linkage getFormalLinkage(Linkage L) {
   switch (L) {
   case UniqueExternalLinkage:
     return ExternalLinkage;
   case VisibleNoLinkage:
     return NoLinkage;
   case ModuleInternalLinkage:
     return InternalLinkage;
   default:
     return L;
   }
 }
  
 inline bool isExternalFormalLinkage(Linkage L) {
   return getFormalLinkage(L) == ExternalLinkage;
 }
  
 /// Compute the minimum linkage given two linkages.
 ///
 /// The linkage can be interpreted as a pair formed by the formal linkage and
 /// a boolean for external visibility. This is just what getFormalLinkage and
 /// isExternallyVisible return. We want the minimum of both components. The
 /// Linkage enum is defined in an order that makes this simple, we just need
 /// special cases for when VisibleNoLinkage would lose the visible bit and
 /// become NoLinkage.
 inline Linkage minLinkage(Linkage L1, Linkage L2) {
   if (L2 == VisibleNoLinkage)
     std::swap(L1, L2);
   if (L1 == VisibleNoLinkage) {
     if (L2 == InternalLinkage)
       return NoLinkage;
     if (L2 == UniqueExternalLinkage)
       return NoLinkage;
   }
   return L1 < L2 ? L1 : L2;
 }
enum Visibility {
   /// Objects with "hidden" visibility are not seen by the dynamic
   /// linker.
   HiddenVisibility,
  
   /// Objects with "protected" visibility are seen by the dynamic
   /// linker but always dynamically resolve to an object within this
   /// shared object.
   ProtectedVisibility,
  
   /// Objects with "default" visibility are seen by the dynamic linker
   /// and act like normal objects.
   DefaultVisibility
 };
  
 inline Visibility minVisibility(Visibility L, Visibility R) {
   return L < R ? L : R;
 }
  
 class LinkageInfo {
   uint8_t linkage_    : 3;
   uint8_t visibility_ : 2;
   uint8_t explicit_   : 1;
  
   void setVisibility(Visibility V, bool E) { visibility_ = V; explicit_ = E; }
 public:
   LinkageInfo() : linkage_(ExternalLinkage), visibility_(DefaultVisibility),
                   explicit_(false) {}
   LinkageInfo(Linkage L, Visibility V, bool E)
     : linkage_(L), visibility_(V), explicit_(E) {
     assert(getLinkage() == L && getVisibility() == V &&
            isVisibilityExplicit() == E && "Enum truncated!");
   }
  
   static LinkageInfo external() {
     return LinkageInfo();
   }
   static LinkageInfo internal() {
     return LinkageInfo(InternalLinkage, DefaultVisibility, false);
   }
   static LinkageInfo uniqueExternal() {
     return LinkageInfo(UniqueExternalLinkage, DefaultVisibility, false);
   }
   static LinkageInfo none() {
     return LinkageInfo(NoLinkage, DefaultVisibility, false);
   }
   static LinkageInfo visible_none() {
     return LinkageInfo(VisibleNoLinkage, DefaultVisibility, false);
   }
  
   Linkage getLinkage() const { return (Linkage)linkage_; }
   Visibility getVisibility() const { return (Visibility)visibility_; }
   bool isVisibilityExplicit() const { return explicit_; }
  
   void setLinkage(Linkage L) { linkage_ = L; }
  
   void mergeLinkage(Linkage L) {
     setLinkage(minLinkage(getLinkage(), L));
   }
   void mergeLinkage(LinkageInfo other) {
     mergeLinkage(other.getLinkage());
   }
  
   void mergeExternalVisibility(Linkage L) {
     Linkage ThisL = getLinkage();
     if (!isExternallyVisible(L)) {
       if (ThisL == VisibleNoLinkage)
         ThisL = NoLinkage;
       else if (ThisL == ExternalLinkage)
         ThisL = UniqueExternalLinkage;
     }
     setLinkage(ThisL);
   }
   void mergeExternalVisibility(LinkageInfo Other) {
     mergeExternalVisibility(Other.getLinkage());
   }
  
   /// Merge in the visibility 'newVis'.
   void mergeVisibility(Visibility newVis, bool newExplicit) {
     Visibility oldVis = getVisibility();
  
     // Never increase visibility.
     if (oldVis < newVis)
       return;
  
     // If the new visibility is the same as the old and the new
     // visibility isn't explicit, we have nothing to add.
     if (oldVis == newVis && !newExplicit)
       return;
  
     // Otherwise, we're either decreasing visibility or making our
     // existing visibility explicit.
     setVisibility(newVis, newExplicit);
   }
   void mergeVisibility(LinkageInfo other) {
     mergeVisibility(other.getVisibility(), other.isVisibilityExplicit());
   }
  
   /// Merge both linkage and visibility.
   void merge(LinkageInfo other) {
     mergeLinkage(other);
     mergeVisibility(other);
   }
  
   /// Merge linkage and conditionally merge visibility.
   void mergeMaybeWithVisibility(LinkageInfo other, bool withVis) {
     mergeLinkage(other);
     if (withVis) mergeVisibility(other);
   }
 };
 