
void
Settings::getSelectionColor(double* r,
                            double* g,
                            double* b) const
{
    *r = _selectionColor->getValue(0);
    *g = _selectionColor->getValue(1);
    *b = _selectionColor->getValue(2);
}

void
Settings::getInterpolatedColor(double* r,
                               double* g,
                               double* b) const
{
    *r = _interpolatedColor->getValue(0);
    *g = _interpolatedColor->getValue(1);
    *b = _interpolatedColor->getValue(2);
}

void
Settings::getKeyframeColor(double* r,
                           double* g,
                           double* b) const
{
    *r = _keyframeColor->getValue(0);
    *g = _keyframeColor->getValue(1);
    *b = _keyframeColor->getValue(2);
}

void
Settings::getTrackerKeyframeColor(double* r,
                                  double* g,
                                  double* b) const
{
    *r = _trackerKeyframeColor->getValue(0);
    *g = _trackerKeyframeColor->getValue(1);
    *b = _trackerKeyframeColor->getValue(2);
}

void
Settings::getExprColor(double* r,
                       double* g,
                       double* b) const
{
    *r = _exprColor->getValue(0);
    *g = _exprColor->getValue(1);
    *b = _exprColor->getValue(2);
}

void
Settings::getTextColor(double* r,
                       double* g,
                       double* b) const
{
    *r = _textColor->getValue(0);
    *g = _textColor->getValue(1);
    *b = _textColor->getValue(2);
}

void
Settings::getAltTextColor(double* r,
                          double* g,
                          double* b) const
{
    *r = _altTextColor->getValue(0);
    *g = _altTextColor->getValue(1);
    *b = _altTextColor->getValue(2);
}

void
Settings::getTimelinePlayheadColor(double* r,
                                   double* g,
                                   double* b) const
{
    *r = _timelinePlayheadColor->getValue(0);
    *g = _timelinePlayheadColor->getValue(1);
    *b = _timelinePlayheadColor->getValue(2);
}

void
Settings::getTimelineBoundsColor(double* r,
                                 double* g,
                                 double* b) const
{
    *r = _timelineBoundsColor->getValue(0);
    *g = _timelineBoundsColor->getValue(1);
    *b = _timelineBoundsColor->getValue(2);
}

void
Settings::getTimelineBGColor(double* r,
                             double* g,
                             double* b) const
{
    *r = _timelineBGColor->getValue(0);
    *g = _timelineBGColor->getValue(1);
    *b = _timelineBGColor->getValue(2);
}

void
Settings::getCachedFrameColor(double* r,
                              double* g,
                              double* b) const
{
    *r = _cachedFrameColor->getValue(0);
    *g = _cachedFrameColor->getValue(1);
    *b = _cachedFrameColor->getValue(2);
}

void
Settings::getDiskCachedColor(double* r,
                             double* g,
                             double* b) const
{
    *r = _diskCachedFrameColor->getValue(0);
    *g = _diskCachedFrameColor->getValue(1);
    *b = _diskCachedFrameColor->getValue(2);
}

void
Settings::getCurveEditorBGColor(double* r,
                                double* g,
                                double* b) const
{
    *r = _curveEditorBGColor->getValue(0);
    *g = _curveEditorBGColor->getValue(1);
    *b = _curveEditorBGColor->getValue(2);
}

void
Settings::getCurveEditorGridColor(double* r,
                                  double* g,
                                  double* b) const
{
    *r = _gridColor->getValue(0);
    *g = _gridColor->getValue(1);
    *b = _gridColor->getValue(2);
}

void
Settings::getCurveEditorScaleColor(double* r,
                                   double* g,
                                   double* b) const
{
    *r = _curveEditorScaleColor->getValue(0);
    *g = _curveEditorScaleColor->getValue(1);
    *b = _curveEditorScaleColor->getValue(2);
}

void
Settings::getDopeSheetEditorBackgroundColor(double *r,
                                            double *g,
                                            double *b) const
{
    *r = _dopeSheetEditorBackgroundColor->getValue(0);
    *g = _dopeSheetEditorBackgroundColor->getValue(1);
    *b = _dopeSheetEditorBackgroundColor->getValue(2);
}

void
Settings::getDopeSheetEditorRootRowBackgroundColor(double *r,
                                                   double *g,
                                                   double *b,
                                                   double *a) const
{
    *r = _dopeSheetEditorRootSectionBackgroundColor->getValue(0);
    *g = _dopeSheetEditorRootSectionBackgroundColor->getValue(1);
    *b = _dopeSheetEditorRootSectionBackgroundColor->getValue(2);
    *a = _dopeSheetEditorRootSectionBackgroundColor->getValue(3);
}

void
Settings::getDopeSheetEditorKnobRowBackgroundColor(double *r,
                                                   double *g,
                                                   double *b,
                                                   double *a) const
{
    *r = _dopeSheetEditorKnobSectionBackgroundColor->getValue(0);
    *g = _dopeSheetEditorKnobSectionBackgroundColor->getValue(1);
    *b = _dopeSheetEditorKnobSectionBackgroundColor->getValue(2);
    *a = _dopeSheetEditorKnobSectionBackgroundColor->getValue(3);
}

void
Settings::getDopeSheetEditorScaleColor(double *r,
                                       double *g,
                                       double *b) const
{
    *r = _dopeSheetEditorScaleColor->getValue(0);
    *g = _dopeSheetEditorScaleColor->getValue(1);
    *b = _dopeSheetEditorScaleColor->getValue(2);
}

void
Settings::getDopeSheetEditorGridColor(double *r,
                                      double *g,
                                      double *b) const
{
    *r = _dopeSheetEditorGridColor->getValue(0);
    *g = _dopeSheetEditorGridColor->getValue(1);
    *b = _dopeSheetEditorGridColor->getValue(2);
}

void
Settings::getSEKeywordColor(double* r,
                            double* g,
                            double* b) const
{
    *r = _keywordColor->getValue(0);
    *g = _keywordColor->getValue(1);
    *b = _keywordColor->getValue(2);
}

void
Settings::getSEOperatorColor(double* r,
                             double* g,
                             double* b) const
{
    *r = _operatorColor->getValue(0);
    *g = _operatorColor->getValue(1);
    *b = _operatorColor->getValue(2);
}

void
Settings::getSEBraceColor(double* r,
                          double* g,
                          double* b) const
{
    *r = _braceColor->getValue(0);
    *g = _braceColor->getValue(1);
    *b = _braceColor->getValue(2);
}

void
Settings::getSEDefClassColor(double* r,
                             double* g,
                             double* b) const
{
    *r = _defClassColor->getValue(0);
    *g = _defClassColor->getValue(1);
    *b = _defClassColor->getValue(2);
}

void
Settings::getSEStringsColor(double* r,
                            double* g,
                            double* b) const
{
    *r = _stringsColor->getValue(0);
    *g = _stringsColor->getValue(1);
    *b = _stringsColor->getValue(2);
}

void
Settings::getSECommentsColor(double* r,
                             double* g,
                             double* b) const
{
    *r = _commentsColor->getValue(0);
    *g = _commentsColor->getValue(1);
    *b = _commentsColor->getValue(2);
}

void
Settings::getSESelfColor(double* r,
                         double* g,
                         double* b) const
{
    *r = _selfColor->getValue(0);
    *g = _selfColor->getValue(1);
    *b = _selfColor->getValue(2);
}

void
Settings::getSENumbersColor(double* r,
                            double* g,
                            double* b) const
{
    *r = _numbersColor->getValue(0);
    *g = _numbersColor->getValue(1);
    *b = _numbersColor->getValue(2);
}

void
Settings::getSECurLineColor(double* r,
                            double* g,
                            double* b) const
{
    *r = _curLineColor->getValue(0);
    *g = _curLineColor->getValue(1);
    *b = _curLineColor->getValue(2);
}

void
Settings::getSliderColor(double* r,
                            double* g,
                            double* b) const
{
    *r = _sliderColor->getValue(0);
    *g = _sliderColor->getValue(1);
    *b = _sliderColor->getValue(2);
}

int
Settings::getSEFontSize() const
{
    return _scriptEditorFontSize->getValue();
}

std::string
Settings::getSEFontFamily() const
{
    return _scriptEditorFontChoice->getActiveEntry().id;
}

void
Settings::getPluginIconFrameColor(int *r,
                                  int *g,
                                  int *b) const
{
    *r = 50;
    *g = 50;
    *b = 50;
}

int
Settings::getDopeSheetEditorNodeSeparationWith() const
{
    return 4;
}

bool
Settings::isNaNHandlingEnabled() const
{
    return _convertNaNValues->getValue();
}

bool
Settings::isCopyInputImageForPluginRenderEnabled() const
{
    return _pluginUseImageCopyForSource->getValue();
}

void
Settings::setOnProjectCreatedCB(const std::string& func)
{
    _onProjectCreated->setValue(func);
}

void
Settings::setOnProjectLoadedCB(const std::string& func)
{
    _defaultOnProjectLoaded->setValue(func);
}

bool
Settings::isDefaultAppearanceOutdated() const
{
    return _defaultAppearanceOutdated;
}

void
Settings::restoreDefaultAppearance()
{
    std::vector<KnobIPtr> children = _appearanceTab->getChildren();

    for (std::size_t i = 0; i < children.size(); ++i) {
        KnobColor* isColorKnob = dynamic_cast<KnobColor*>( children[i].get() );
        if ( isColorKnob && isColorKnob->isSimplified() ) {
            isColorKnob->blockValueChanges();
            for (int j = 0; j < isColorKnob->getDimension(); ++j) {
                isColorKnob->resetToDefaultValue(j);
            }
            isColorKnob->unblockValueChanges();
        }
    }
    _defaultAppearanceOutdated = false;
    appPTR->reloadStylesheets();
}

std::string
Settings::getUserStyleSheetFilePath() const
{
    return _qssFile->getValue();
}

void
Settings::setRenderQueuingEnabled(bool enabled)
{
    _queueRenders->setValue(enabled);
    saveSetting( _queueRenders.get() );
}

bool
Settings::isRenderQueuingEnabled() const
{
    return _queueRenders->getValue();
}

bool
Settings::isFileDialogEnabledForNewWriters() const
{
#ifdef NATRON_ENABLE_IO_META_NODES

    return _filedialogForWriters->getValue();
#else

    return true;
#endif
}

bool
Settings::isDriveLetterToUNCPathConversionEnabled() const
{
    return !_enableMappingFromDriveLettersToUNCShareNames->getValue();
}

NATRON_NAMESPACE_EXIT

NATRON_NAMESPACE_USING
#include "moc_Settings.cpp"
