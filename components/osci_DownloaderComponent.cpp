#include "osci_DownloaderComponent.h"

DownloaderComponent::DownloaderComponent() : juce::Thread("DownloaderComponent") {
    addAndMakeVisible(progress_);
    progress_.reset();
}

void DownloaderComponent::setup(juce::URL url, juce::File file) {
    this->url = url;
    this->file = file;

    if (url.toString(false).endsWithIgnoreCase(".gz")) {
        uncompressOnFinish = true;
        this->file = file.getSiblingFile(file.getFileName() + ".gz");
    }
}

void DownloaderComponent::run() {
    downloader = std::make_unique<DownloaderThread>(url, file, this, taskLock, task);
    downloader->startThread();
    while (!threadShouldExit()) {
        {
            juce::CriticalSection::ScopedTryLockType lock(taskLock);
            if (lock.isLocked()) {
                // JUCE gives no task at all when it cannot connect.
                const auto connectionFailed = task == nullptr && !downloader->isThreadRunning();
                if (connectionFailed || (task != nullptr && task->isFinished())) {
                    break;
                }
            }
        }
        wait(100);
    }
    threadComplete();
}

void DownloaderComponent::threadComplete() {
    bool error = task == nullptr || task->hadError() || !task->isFinished();
    if (uncompressOnFinish && !error) {
        juce::FileInputStream input(file);
        juce::GZIPDecompressorInputStream decompressedInput(&input, false, juce::GZIPDecompressorInputStream::gzipFormat);
        juce::File uncompressedFile = file.getSiblingFile(file.getFileNameWithoutExtension());
        juce::FileOutputStream output(uncompressedFile);
        if (output.writeFromInputStream(decompressedInput, -1) < 1) {
            uncompressedFile.deleteFile();
            error = true;
        } else {
            uncompressedFile.setExecutePermission(true);
        }
        file.deleteFile();
    }

    if (error) {
        file.deleteFile();
        progress_.showError("Download failed");
        report([](DownloaderComponent& self) { if (self.onFailedDownload != nullptr) { self.onFailedDownload(); } });
    } else {
        if (onSuccessfulDownload != nullptr) {
            onSuccessfulDownload();
        }
        progress_.showSuccess(file.getFileName() + " downloaded!");
    }
}

void DownloaderComponent::report(std::function<void(DownloaderComponent&)> call) {
    juce::Component::SafePointer<DownloaderComponent> self(this);
    juce::MessageManager::callAsync([self, call = std::move(call)] {
        if (self != nullptr) {
            call(*self);
        }
    });
}

void DownloaderComponent::resized() {
    progress_.setBounds(getLocalBounds());
}

void DownloaderComponent::finished(juce::URL::DownloadTask* task, bool success) {
    notify();
}

void DownloaderComponent::progress(juce::URL::DownloadTask* task, juce::int64 bytesDownloaded, juce::int64 totalLength) {
    // Reserve the last 10% for post-download decompression so the bar doesn't
    // sit at 100% while we're still working.
    const double fraction = totalLength <= 0 ? -1.0 : (uncompressOnFinish ? 0.9 : 1.0) * (double) bytesDownloaded / (double) totalLength;
    progress_.setProgress(fraction);
    const int percent = fraction < 0.0 ? -1 : (int) (fraction * 100.0);
    if (percent != reportedPercent) {
        reportedPercent = percent;
        report([fraction](DownloaderComponent& self) { if (self.onProgress != nullptr) { self.onProgress(fraction); } });
    }
}

void DownloaderComponent::download() {
    reportedPercent = -2;
    progress_.reset();
    progress_.setStatus("Downloading...");
    progress_.setProgress(-1.0);
    startThread();
}
