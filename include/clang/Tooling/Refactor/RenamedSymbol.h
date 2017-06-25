//===--- RenamedSymbol.h - ---------------------------------*- C++ -*------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLING_REFACTOR_RENAMED_SYMBOL_H
#define LLVM_CLANG_TOOLING_REFACTOR_RENAMED_SYMBOL_H

#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Tooling/Refactor/SymbolName.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSet.h"

namespace clang {

class NamedDecl;

namespace tooling {
namespace rename {

/// \brief A symbol that has to be renamed.
class Symbol {
public:
  SymbolName Name;
  /// The index of this symbol in a \c SymbolOperation.
  unsigned SymbolIndex;
  /// The declaration that was used to initiate a refactoring operation for this
  /// symbol. May not be the most canonical declaration.
  const NamedDecl *FoundDecl;
  /// An optional Objective-C selector.
  llvm::Optional<Selector> ObjCSelector;

  Symbol(const NamedDecl *FoundDecl, unsigned SymbolIndex,
         const LangOptions &LangOpts);

  Symbol(Symbol &&) = default;
  Symbol &operator=(Symbol &&) = default;
};

/// \brief An occurrence of a renamed symbol.
///
/// Provides information about an occurrence of symbol that helps renaming tools
/// determine if they can rename this symbol automatically and which source
/// ranges they have to replace.
///
/// A single occurrence of a symbol can span more than one source range to
/// account for things like Objective-C selectors.
// TODO: Rename
class SymbolOccurrence {
public:
  enum OccurrenceKind {
    /// \brief This occurrence is an exact match and can be renamed
    /// automatically.
    MatchingSymbol,

    /// \brief This is an occurrence of a matching selector. It can't be renamed
    /// automatically unless the indexer proves that this selector refers only
    /// to the declarations that correspond to the renamed symbol.
    MatchingSelector,

    /// \brief This is an occurrence of an implicit property that uses the
    /// renamed method.
    MatchingImplicitProperty,

    /// \brief This is a textual occurrence of a symbol in a comment.
    MatchingComment,

    /// \brief This is a textual occurrence of a symbol in a doc comment.
    MatchingDocComment,

    /// \brief This is an occurrence of a symbol in an inclusion directive.
    MatchingFilename
  };

  OccurrenceKind Kind;
  /// Whether or not this occurrence is inside a macro. When this is true, the
  /// locations of the occurrence contain just one location that points to
  /// the location of the macro expansion.
  bool IsMacroExpansion;
  /// The index of the symbol stored in a \c SymbolOperation which matches this
  /// occurrence.
  unsigned SymbolIndex;
  /// The source locations that correspond to the occurence of the symbol.
  SmallVector<SourceLocation, 4> Locations;

  SymbolOccurrence()
      : Kind(MatchingSymbol), IsMacroExpansion(false), SymbolIndex(0) {}

  SymbolOccurrence(OccurrenceKind Kind, bool IsMacroExpansion,
                   unsigned SymbolIndex, ArrayRef<SourceLocation> Locations)
      : Kind(Kind), IsMacroExpansion(IsMacroExpansion),
        SymbolIndex(SymbolIndex),
        Locations(Locations.begin(), Locations.end()) {
    assert(!Locations.empty() && "Renamed occurence without locations!");
  }

  SymbolOccurrence(SymbolOccurrence &&) = default;
  SymbolOccurrence &operator=(SymbolOccurrence &&) = default;

  /// Return the source range that corresponds to an individual source location
  /// in this occurrence.
  SourceRange getLocationRange(SourceLocation Loc, size_t OldNameSize) const {
    return SourceRange(
        Loc, IsMacroExpansion ? Loc : Loc.getLocWithOffset(OldNameSize));
  }
};

/// \brief Less-than operator between the two renamed symbol occurrences.
bool operator<(const SymbolOccurrence &LHS, const SymbolOccurrence &RHS);

/// \brief Equal-to operator between the two renamed symbol occurrences.
bool operator==(const SymbolOccurrence &LHS, const SymbolOccurrence &RHS);

} // end namespace rename
} // end namespace tooling
} // end namespace clang

#endif // LLVM_CLANG_TOOLING_REFACTOR_RENAMED_SYMBOL_H