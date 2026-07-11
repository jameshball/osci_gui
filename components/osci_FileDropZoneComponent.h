#pragma once

namespace osci {

class FileDropZoneComponent final : public juce::Component,
                                    public juce::FileDragAndDropTarget,
                                    private juce::Timer {
public:
    enum class Status {
        empty,
        dragAccept,
        dragReject,
        selected,
        busy,
        success,
        warning,
        error,
        disabled
    };

    FileDropZoneComponent();
    ~FileDropZoneComponent() override = default;

    void setTitle (juce::String text);
    void setSubtitle (juce::String text);
    void setActionText (juce::String text);
    void setSelectedFile (juce::File file, juce::String detail = {});
    void clearSelectedFile();
    void setStatus (Status newStatus, juce::String message = {});
    void setAcceptedDescription (juce::String text);
    void setIsFileAccepted (std::function<bool (const juce::File&)> predicate);
    void setAccentColour (juce::Colour colour);
    void clearAccentColour();

    std::function<void()> onBrowseRequested;
    std::function<void (std::vector<juce::File>)> onFilesDropped;
    std::function<void (std::vector<juce::File>)> onRejectedFiles;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragMove (const juce::StringArray& files, int x, int y) override;
    void fileDragExit (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void lookAndFeelChanged() override;
    void enablementChanged() override;
    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;

private:
    Status getEffectiveStatus() const;
    bool canAcceptDrops() const;
    bool acceptsFile (const juce::File& file) const;
    std::vector<juce::File> makeFiles (const juce::StringArray& paths) const;
    std::vector<juce::File> acceptedFilesFrom (const std::vector<juce::File>& files) const;
    std::vector<juce::File> rejectedFilesFrom (const std::vector<juce::File>& files) const;
    void updateDragState (const juce::StringArray& files);
    void clearDragState();
    void triggerDropPulse();
    void triggerRejectNudge();
    void updateActionButton();
    void updateAnimationTimer();
    juce::Colour getAccentColour() const;
    void timerCallback() override;

    juce::String title = "Drop files here";
    juce::String subtitle = "or choose one from disk";
    juce::String actionText = "Choose File...";
    juce::String acceptedDescription = "file";
    juce::String statusMessage;
    juce::File selectedFile;
    juce::String selectedDetail;
    bool hasSelectedFile = false;
    bool mouseWasDownInside = false;
    bool dragActive = false;
    bool dragAccepted = false;
    Status status = Status::empty;
    float busyPhase = 0.0f;
    std::optional<juce::Colour> customAccentColour;

    std::function<bool (const juce::File&)> isFileAccepted;
    juce::TextButton actionButton;
    ToggleAnimationController hoverAnimation;
    ToggleAnimationController dragAnimation;
    ToggleAnimationController pulseAnimation;
    ToggleAnimationController rejectAnimation;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileDropZoneComponent)
};

} // namespace osci
