#ifndef TTGAMMAEVENTSAVER_TTGAMAEVENTSAVERLOADER_H_
#define TTGAMMAEVENTSAVER_TTGAMAEVENTSAVERLOADER_H_

#include "TopEventSelectionTools/ToolLoaderBase.h"

/**
 * The name of this class has to be $packagename + Loader. So if
 * your package is called foo, you'll need a fooLoader class.
 * This allows us to write custom event selection tools
 */
namespace top {
class TTGammaEventSaverLoader : public ToolLoaderBase {
 public:
  top::EventSelectorBase* initTool(const std::string& name, const std::string& line, TFile* outputFile, std::shared_ptr<top::TopConfig> config,EL::Worker* wk = nullptr);

  ClassDef(top::TTGammaEventSaverLoader, 0)
};
}  // namespace top

#endif  // TTGAMMAEVENTSAVER_TTGAMAEVENTSAVERLOADER_H_
