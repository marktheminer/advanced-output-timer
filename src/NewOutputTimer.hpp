#pragma once

#include <QDialog>
#include <memory>

#include "ui_new-output-timer.h"

class QCloseEvent;

class NewOutputTimer : public QDialog {
	Q_OBJECT

public:
	std::unique_ptr<Ui_NewOutputTimer> ui;
	NewOutputTimer(QWidget *parent);

	void closeEvent(QCloseEvent *event) override;
	void PauseRecordingTimer();
	void UnpauseRecordingTimer();

public slots:
	void StreamingTimerButton();
	void StartRecordingTimerButton();
	void RecordingTimerButton();
	void StreamTimerStart();
	void StartRecordTimerStart();
	void RecordTimerStart();
	void StreamTimerStop();
	void StartRecordTimerStop();
	void RecordTimerStop();
	void UpdateStreamTimerDisplay();
	void UpdateRecordStartTimerDisplay();
	void UpdateRecordStopTimerDisplay();
	void ShowHideDialog();
	void EventStopStreaming();
	void EventStartRecording();
	void EventStopRecording();

private:
	bool streamingAlreadyActive = false;
	bool recordingAlreadyActive = false;
	bool stoppingStreamTimer = false;
	bool stoppingStartRecordingTimer = false;
	bool stoppingRecordingTimer = false;

	QTimer *streamingTimer;
	QTimer *recordingStartTimer;
	QTimer *recordingStopTimer;
	QTimer *streamingTimerDisplay;
	QTimer *recordingStartTimerDisplay;
	QTimer *recordingStopTimerDisplay;

	int recordingTimeLeft;
};
