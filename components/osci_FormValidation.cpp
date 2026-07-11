#include "osci_FormValidation.h"

namespace osci {
namespace {
bool isAsciiLetter(juce::juce_wchar character) {
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z');
}

bool isAsciiDigit(juce::juce_wchar character) {
    return character >= '0' && character <= '9';
}

bool isAsciiLetterOrDigit(juce::juce_wchar character) {
    return isAsciiLetter(character) || isAsciiDigit(character);
}
} // namespace

StringValidation& StringValidation::trim() {
    trimsWhitespace = true;
    return *this;
}

StringValidation& StringValidation::required(juce::String message) {
    return custom("required", std::move(message), [](const juce::String& value) { return value.isNotEmpty(); });
}

StringValidation& StringValidation::email(juce::String message) {
    return custom("email", std::move(message), [](const juce::String& value) { return isValidEmail(value); });
}

StringValidation& StringValidation::minLength(int length, juce::String message) {
    jassert(length >= 0);
    length = juce::jmax(0, length);
    return custom("min_length", std::move(message), [length](const juce::String& value) { return value.length() >= length; });
}

StringValidation& StringValidation::maxLength(int length, juce::String message) {
    jassert(length >= 0);
    length = juce::jmax(0, length);
    return custom("max_length", std::move(message), [length](const juce::String& value) { return value.length() <= length; });
}

StringValidation& StringValidation::custom(juce::String code, juce::String message, Predicate predicate) {
    jassert(code.isNotEmpty());
    jassert(predicate != nullptr);
    if (code.isEmpty() || predicate == nullptr) {
        return *this;
    }
    rules.push_back({ std::move(code), std::move(message), std::move(predicate) });
    return *this;
}

std::optional<ValidationError> StringValidation::validate(juce::String value) const {
    if (trimsWhitespace) {
        value = value.trim();
    }

    for (const auto& rule : rules) {
        if (rule.predicate != nullptr && !rule.predicate(value)) {
            return ValidationError { rule.code, rule.message };
        }
    }

    return std::nullopt;
}

bool StringValidation::isValidEmail(juce::StringRef valueRef) {
    const auto value = juce::String(valueRef);
    if (value.isEmpty() || value.length() > 254 || value.containsAnyOf(" \t\r\n")) {
        return false;
    }

    const auto at = value.indexOfChar('@');
    if (at <= 0 || at != value.lastIndexOfChar('@') || at >= value.length() - 1) {
        return false;
    }

    const auto local = value.substring(0, at);
    const auto domain = value.substring(at + 1);
    if (local.length() > 64 || local.startsWithChar('.') || local.endsWithChar('.') || local.contains("..")) {
        return false;
    }

    const juce::String allowedLocalPunctuation("._'+-");
    for (const auto character : local) {
        if (!isAsciiLetterOrDigit(character) && !allowedLocalPunctuation.containsChar(character)) {
            return false;
        }
    }
    const auto finalLocalCharacter = local.getLastCharacter();
    if (!isAsciiLetterOrDigit(finalLocalCharacter) && !juce::String("_+-").containsChar(finalLocalCharacter)) {
        return false;
    }

    const auto labels = juce::StringArray::fromTokens(domain, ".", {});
    if (labels.size() < 2) {
        return false;
    }

    for (const auto& label : labels) {
        if (label.isEmpty() || label.length() > 63 || label.startsWithChar('-') || label.endsWithChar('-')) {
            return false;
        }

        for (const auto character : label) {
            if (!isAsciiLetterOrDigit(character) && character != '-') {
                return false;
            }
        }
    }

    const auto topLevelDomain = labels[labels.size() - 1];
    if (topLevelDomain.length() < 2) {
        return false;
    }

    for (const auto character : topLevelDomain) {
        if (!isAsciiLetter(character)) {
            return false;
        }
    }

    return true;
}

bool FormValidator::Result::isValid() const {
    return issues.empty();
}

FormValidator::Result::operator bool() const {
    return isValid();
}

int FormValidator::Result::getIssueCount() const {
    return static_cast<int>(issues.size());
}

const FormValidator::Issue* FormValidator::Result::getFirstIssue() const {
    return issues.empty() ? nullptr : &issues.front();
}

const std::vector<FormValidator::Issue>& FormValidator::Result::getIssues() const {
    return issues;
}

FormValidator::FormValidator()
    : FormValidator(Options {}) {}

FormValidator::FormValidator(Options optionsToUse)
    : options(std::move(optionsToUse)) {}

FormValidator::~FormValidator() {
    for (const auto& field : fields) {
        if (field->editor != nullptr) {
            field->editor->removeListener(this);
        }
    }
}

void FormValidator::registerField(juce::String name, juce::TextEditor& editor, StringValidation validation) {
    jassert(name.isNotEmpty());
    jassert(findField(name) == nullptr);
    jassert(findField(editor) == nullptr);
    if (name.isEmpty() || findField(name) != nullptr || findField(editor) != nullptr) {
        return;
    }

    auto field = std::make_unique<Field>();
    field->name = std::move(name);
    field->editor = &editor;
    field->validation = std::move(validation);
    field->normalOutlineColour = editor.findColour(juce::TextEditor::outlineColourId);
    field->focusedOutlineColour = editor.findColour(juce::TextEditor::focusedOutlineColourId);
    editor.addListener(this);
    fields.push_back(std::move(field));
}

void FormValidator::unregisterField(juce::TextEditor& editor) {
    const auto match = [&editor](const std::unique_ptr<Field>& field) { return field->editor == &editor; };
    const auto iterator = std::find_if(fields.begin(), fields.end(), match);
    if (iterator == fields.end()) {
        return;
    }

    editor.removeListener(this);
    fields.erase(iterator);
    notifyValidationChanged();
}

FormValidator::Result FormValidator::validate(bool focusFirstInvalid) {
    submitted = true;
    for (const auto& field : fields) {
        validateField(*field);
    }

    auto result = getResult();
    if (focusFirstInvalid) {
        const auto* firstIssue = result.getFirstIssue();
        if (firstIssue != nullptr && firstIssue->component != nullptr) {
            firstIssue->component->grabKeyboardFocus();
        }
    }

    notifyValidationChanged();
    return result;
}

FormValidator::Result FormValidator::getResult() const {
    Result result;
    for (const auto& field : fields) {
        if (field->state.error.has_value()) {
            result.issues.push_back({ field->name, *field->state.error, field->editor });
        }
    }
    return result;
}

std::optional<FormValidator::FieldState> FormValidator::getFieldState(juce::StringRef name) const {
    const auto* field = findField(name);
    return field != nullptr ? std::optional<FieldState>(field->state) : std::nullopt;
}

bool FormValidator::setError(juce::StringRef name, ValidationError error, bool focus) {
    auto* field = findField(name);
    if (field == nullptr) {
        return false;
    }

    field->state.validated = true;
    field->state.error = std::move(error);
    applyFieldStyle(*field);
    if (focus && field->editor != nullptr) {
        field->editor->grabKeyboardFocus();
    }
    notifyValidationChanged();
    return true;
}

bool FormValidator::clearError(juce::StringRef name) {
    auto* field = findField(name);
    if (field == nullptr) {
        return false;
    }

    field->state.error.reset();
    applyFieldStyle(*field);
    notifyValidationChanged();
    return true;
}

void FormValidator::resetValidation() {
    submitted = false;
    for (const auto& field : fields) {
        field->state = {};
        applyFieldStyle(*field);
    }
    notifyValidationChanged();
}

bool FormValidator::hasBeenSubmitted() const {
    return submitted;
}

void FormValidator::textEditorTextChanged(juce::TextEditor& editor) {
    auto* field = findField(editor);
    if (field == nullptr) {
        return;
    }

    field->state.dirty = true;
    if (currentMode() == Mode::onChange) {
        validateField(*field);
        notifyValidationChanged();
    }
}

void FormValidator::textEditorFocusLost(juce::TextEditor& editor) {
    auto* field = findField(editor);
    if (field == nullptr) {
        return;
    }

    field->state.touched = true;
    if (currentMode() == Mode::onBlur) {
        validateField(*field);
        notifyValidationChanged();
    }
}

FormValidator::Field* FormValidator::findField(juce::TextEditor& editor) {
    const auto match = [&editor](const std::unique_ptr<Field>& field) { return field->editor == &editor; };
    const auto iterator = std::find_if(fields.begin(), fields.end(), match);
    return iterator != fields.end() ? iterator->get() : nullptr;
}

FormValidator::Field* FormValidator::findField(juce::StringRef name) {
    const auto match = [name](const std::unique_ptr<Field>& field) { return field->name == name; };
    const auto iterator = std::find_if(fields.begin(), fields.end(), match);
    return iterator != fields.end() ? iterator->get() : nullptr;
}

const FormValidator::Field* FormValidator::findField(juce::StringRef name) const {
    const auto match = [name](const std::unique_ptr<Field>& field) { return field->name == name; };
    const auto iterator = std::find_if(fields.begin(), fields.end(), match);
    return iterator != fields.end() ? iterator->get() : nullptr;
}

void FormValidator::validateField(Field& field) {
    field.state.validated = true;
    field.state.error = field.validation.validate(field.editor != nullptr ? field.editor->getText() : juce::String());
    applyFieldStyle(field);
}

void FormValidator::applyFieldStyle(Field& field) {
    if (field.editor == nullptr) {
        return;
    }

    const auto invalid = field.state.error.has_value();
    field.editor->setColour(juce::TextEditor::outlineColourId, invalid ? options.errorColour : field.normalOutlineColour);
    field.editor->setColour(juce::TextEditor::focusedOutlineColourId, invalid ? options.errorColour : field.focusedOutlineColour);
    field.editor->repaint();
}

void FormValidator::notifyValidationChanged() {
    if (onValidationChanged != nullptr) {
        onValidationChanged(getResult());
    }
}

FormValidator::Mode FormValidator::currentMode() const {
    return submitted ? options.revalidateMode : options.initialMode;
}

} // namespace osci
