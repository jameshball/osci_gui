#pragma once

namespace osci {

struct ValidationError {
    juce::String code;
    juce::String message;
};

class StringValidation {
public:
    using Predicate = std::function<bool(const juce::String&)>;

    StringValidation& trim();
    StringValidation& required(juce::String message = "This field is required.");
    StringValidation& email(juce::String message = "Enter a valid email address.");
    StringValidation& minLength(int length, juce::String message);
    StringValidation& maxLength(int length, juce::String message);
    StringValidation& custom(juce::String code, juce::String message, Predicate predicate);

    std::optional<ValidationError> validate(juce::String value) const;

    static bool isValidEmail(juce::StringRef value);

private:
    struct Rule {
        juce::String code;
        juce::String message;
        Predicate predicate;
    };

    std::vector<Rule> rules;
    bool trimsWhitespace = false;
};

class FormValidator final : private juce::TextEditor::Listener {
public:
    enum class Mode {
        onSubmit,
        onBlur,
        onChange,
    };

    struct Options {
        Mode initialMode = Mode::onSubmit;
        Mode revalidateMode = Mode::onChange;
        juce::Colour errorColour { Colours::danger() };
    };

    struct FieldState {
        bool dirty = false;
        bool touched = false;
        bool validated = false;
        std::optional<ValidationError> error;
    };

    struct Issue {
        juce::String field;
        ValidationError error;
        juce::Component* component = nullptr;
    };

    class Result {
    public:
        bool isValid() const;
        explicit operator bool() const;
        int getIssueCount() const;
        const Issue* getFirstIssue() const;
        const std::vector<Issue>& getIssues() const;

    private:
        friend class FormValidator;
        std::vector<Issue> issues;
    };

    FormValidator();
    explicit FormValidator(Options options);
    ~FormValidator() override;

    void registerField(juce::String name, juce::TextEditor& editor, StringValidation validation);
    void unregisterField(juce::TextEditor& editor);
    Result validate(bool focusFirstInvalid = true);
    Result getResult() const;
    std::optional<FieldState> getFieldState(juce::StringRef name) const;
    bool setError(juce::StringRef name, ValidationError error, bool focus = false);
    bool clearError(juce::StringRef name);
    void resetValidation();
    bool hasBeenSubmitted() const;

    std::function<void(const Result&)> onValidationChanged;

private:
    struct Field {
        juce::String name;
        juce::TextEditor* editor = nullptr;
        StringValidation validation;
        juce::Colour normalOutlineColour;
        juce::Colour focusedOutlineColour;
        FieldState state;
    };

    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;
    Field* findField(juce::TextEditor& editor);
    Field* findField(juce::StringRef name);
    const Field* findField(juce::StringRef name) const;
    void validateField(Field& field);
    void applyFieldStyle(Field& field);
    void notifyValidationChanged();
    Mode currentMode() const;

    Options options;
    std::vector<std::unique_ptr<Field>> fields;
    bool submitted = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormValidator)
};

} // namespace osci
