/*
AlignmentTraversor.cpp
 JP Robichaud (jp@rev.com)
 2021

*/
#include "AlignmentTraversor.h"

AlignmentTraversor::AlignmentTraversor(wer_alignment &topLevel) : root(topLevel) {
  currentPosInRoot = -1;
  currentSubclass = nullptr;
}

void AlignmentTraversor::Restart() {
  currentPosInRoot = -1;
  currentSubclass = nullptr;
  currentPosInSubclass = -1;
}

bool AlignmentTraversor::NextTriple(triple &triple) {
  if (currentSubclass == nullptr) {
    // we're not in a subclass, we're consuming the root alignment content,
    // let's move to the next word
    currentPosInRoot++;
    if (currentPosInRoot >= root.tokens.size()) {
      return false;
    }

    auto tk = root.tokens[currentPosInRoot];
    if (isEntityLabel(tk.first)) {
      // handle class
      currentPosInSubclass = -1;
      // find subclass spWERA from within the root
      for (auto &a : root.label_alignments) {
        if (a.classLabel == tk.first) {
          currentSubclass = &a;
          break;
        }
      }
      // currentSubclass = nullptr; // fixme
      return NextTriple(triple);
    }

    triple.classLabel = TK_GLOBAL_CLASS;
    triple.ref = tk.first;
    triple.hyp = tk.second;

    return true;
  } else {
    currentPosInSubclass++;
    if (currentPosInSubclass == 0 && currentSubclass->tokens.size() == 0 &&
        currentSubclass->classLabel.find("FALLBACK") != std::string::npos) {
      triple.classLabel = currentSubclass->classLabel;
      triple.ref = NOOP;
      triple.hyp = NOOP;
      return true;
    }
    if (currentPosInSubclass >= currentSubclass->tokens.size()) {
      // we're done here...
      currentSubclass = nullptr;
      currentPosInSubclass = -1;
      return NextTriple(triple);
    }

    auto tk = currentSubclass->tokens[currentPosInSubclass];
    triple.classLabel = currentSubclass->classLabel;
    triple.ref = tk.first;
    triple.hyp = tk.second;
    return true;
  }
}
