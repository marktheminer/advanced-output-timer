#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs.hpp>
#include <util/util.hpp>
#include <QAction>
#include <QMainWindow>
#include <QTimer>
#include <QObject>
#include "NewOutputTimer.hpp"

using namespace std;

NewOutputTimer *ot;

NewOutputTimer::NewOutputTimer(QWidget *parent)
	: QDialog(parent),
	  ui(new Ui_NewOutputTimer)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QObject::connect(ui->outputTimerStream, &QPushButton::clicked, this,
			 &NewOutputTimer::StreamingTimerButton);
	QObject::connect(ui->outputTimerRecordStart, &QPushButton::clicked,
			 this, &NewOutputTimer::StartRecordingTimerButton);
	QObject::connect(ui->outputTimerRecordStop, &QPushButton::clicked, this,
			 &NewOutputTimer::RecordingTimerButton);
	QObject::connect(ui->buttonBox->button(QDialogButtonBox::Close),
			 &QPushButton::clicked, this, &NewOutputTimer::hide);

	streamingTimer = new QTimer(this);
	streamingTimerDisplay = new QTimer(this);

	recordingStartTimer = new QTimer(this);
	recordingStartTimerDisplay = new QTimer(this);

	recordingStopTimer = new QTimer(this);
	recordingStopTimerDisplay = new QTimer(this);

	QObject::connect(streamingTimer, &QTimer::timeout, this,
			 &NewOutputTimer::EventStopStreaming);
	QObject::connect(streamingTimerDisplay, &QTimer::timeout, this,
			 &NewOutputTimer::UpdateStreamTimerDisplay);
	QObject::connect(recordingStartTimer, &QTimer::timeout, this,
			 &NewOutputTimer::EventStartRecording);
	QObject::connect(recordingStartTimerDisplay, &QTimer::timeout, this,
			 &NewOutputTimer::UpdateRecordStartTimerDisplay);
	QObject::connect(recordingStopTimer, &QTimer::timeout, this,
			 &NewOutputTimer::EventStopRecording);
	QObject::connect(recordingStopTimerDisplay, &QTimer::timeout, this,
			 &NewOutputTimer::UpdateRecordStopTimerDisplay);
}

void NewOutputTimer::closeEvent(QCloseEvent *)
{
	obs_frontend_save();
}

void NewOutputTimer::StreamingTimerButton()
{
	if (!obs_frontend_streaming_active()) {
		blog(LOG_INFO, "Starting stream due to NewOutputTimer");
		obs_frontend_streaming_start();
	} else if (streamingAlreadyActive ||
		   !this->streamingTimer->isActive()) {
		StreamTimerStart();
		streamingAlreadyActive = false;
	} else if (obs_frontend_streaming_active()) {
		stoppingStreamTimer = true;
		StreamTimerStop();
		stoppingStreamTimer = false;
	}
}

void NewOutputTimer::StartRecordingTimerButton()
{
	if (obs_frontend_recording_active())
		return;

	if (!recordingAlreadyActive && !this->recordingStartTimer->isActive()) {
		StartRecordTimerStart();
	} else {
		stoppingStartRecordingTimer = true;
		StartRecordTimerStop();
		stoppingStartRecordingTimer = false;
	}
}

void NewOutputTimer::RecordingTimerButton()
{
	if (!obs_frontend_recording_active()) {
		blog(LOG_INFO, "Starting recording due to NewOutputTimer");
		obs_frontend_recording_start();
	} else if (recordingAlreadyActive ||
		   !this->recordingStopTimer->isActive()) {
		RecordTimerStart();
		recordingAlreadyActive = false;
	} else if (obs_frontend_recording_active()) {
		stoppingRecordingTimer = true;
		RecordTimerStop();
		stoppingRecordingTimer = false;
	}
}

void NewOutputTimer::StreamTimerStart()
{
	if (!isVisible() && ui->autoStartStreamTimer->isChecked() == false) {
		streamingAlreadyActive = true;
		return;
	}

	int hours = ui->streamingTimerHours->value();
	int minutes = ui->streamingTimerMinutes->value();
	int seconds = ui->streamingTimerSeconds->value();

	int total = (((hours * 3600) + (minutes * 60)) + seconds) * 1000;

	if (total == 0)
		total = 1000;

	streamingTimer->setInterval(total);
	streamingTimer->setSingleShot(true);

	streamingTimer->start();
	streamingTimerDisplay->start(1000);
	ui->outputTimerStream->setText(obs_module_text("Stop"));

	UpdateStreamTimerDisplay();

	ui->outputTimerStream->setChecked(true);
}

void NewOutputTimer::StartRecordTimerStart()
{
	if (!isVisible())
		return;

	int hours = ui->recordingStartTimerHours->value();
	int minutes = ui->recordingStartTimerMinutes->value();
	int seconds = ui->recordingStartTimerSeconds->value();

	int total = (((hours * 3600) + (minutes * 60)) + seconds) * 1000;

	if (total == 0)
		total = 1000;

	recordingStartTimer->setInterval(total);
	recordingStartTimer->setSingleShot(true);

	recordingStartTimer->start();
	recordingStartTimerDisplay->start(1000);
	ui->outputTimerRecordStart->setText(obs_module_text("Stop"));

	UpdateRecordStartTimerDisplay();

	ui->outputTimerRecordStart->setChecked(true);
}

void NewOutputTimer::RecordTimerStart()
{
	if (!isVisible() &&
	    ui->autoStartRecordStopTimer->isChecked() == false) {
		recordingAlreadyActive = true;
		return;
	}

	int hours = ui->recordingStopTimerHours->value();
	int minutes = ui->recordingStopTimerMinutes->value();
	int seconds = ui->recordingStopTimerSeconds->value();

	int total = (((hours * 3600) + (minutes * 60)) + seconds) * 1000;

	if (total == 0)
		total = 1000;

	recordingStopTimer->setInterval(total);
	recordingStopTimer->setSingleShot(true);

	recordingStopTimer->start();
	recordingStopTimerDisplay->start(1000);
	ui->outputTimerRecordStop->setText(obs_module_text("Stop"));

	UpdateRecordStopTimerDisplay();

	ui->outputTimerRecordStop->setChecked(true);
}

void NewOutputTimer::StreamTimerStop()
{
	streamingAlreadyActive = false;

	if (!isVisible() && streamingTimer->isActive() == false)
		return;

	if (streamingTimer->isActive())
		streamingTimer->stop();

	ui->outputTimerStream->setText(obs_module_text("Start"));

	if (streamingTimerDisplay->isActive())
		streamingTimerDisplay->stop();

	ui->streamTime->setText("00:00:00");
	ui->outputTimerStream->setChecked(false);
}

void NewOutputTimer::StartRecordTimerStop()
{
	if (!isVisible())
		return;

	if (recordingStartTimer->isActive())
		recordingStartTimer->stop();

	ui->outputTimerRecordStart->setText(obs_module_text("Start"));

	if (recordingStartTimerDisplay->isActive())
		recordingStartTimerDisplay->stop();

	ui->recordStartTime->setText("00:00:00");
	ui->outputTimerRecordStart->setChecked(false);
}

void NewOutputTimer::RecordTimerStop()
{
	recordingAlreadyActive = false;

	if (!isVisible() && recordingStopTimer->isActive() == false)
		return;

	if (recordingStopTimer->isActive())
		recordingStopTimer->stop();

	ui->outputTimerRecordStop->setText(obs_module_text("Start"));

	if (recordingStopTimerDisplay->isActive())
		recordingStopTimerDisplay->stop();

	ui->recordStopTime->setText("00:00:00");
	ui->outputTimerRecordStop->setChecked(false);
}

void NewOutputTimer::UpdateStreamTimerDisplay()
{
	int remainingTime = streamingTimer->remainingTime() / 1000;

	int seconds = remainingTime % 60;
	int minutes = (remainingTime % 3600) / 60;
	int hours = remainingTime / 3600;

	QString text =
		QString::asprintf("%02d:%02d:%02d", hours, minutes, seconds);
	ui->streamTime->setText(text);
}

void NewOutputTimer::UpdateRecordStartTimerDisplay()
{
	int remainingTime = 0;

	remainingTime = recordingStartTimer->remainingTime() / 1000;

	int seconds = remainingTime % 60;
	int minutes = (remainingTime % 3600) / 60;
	int hours = remainingTime / 3600;

	QString text =
		QString::asprintf("%02d:%02d:%02d", hours, minutes, seconds);
	ui->recordStartTime->setText(text);
}

void NewOutputTimer::UpdateRecordStopTimerDisplay()
{
	int remainingTimeBeforeStopRecording = 0;

	if (obs_frontend_recording_paused() &&
	    ui->pauseRecordTimer->isChecked())
		remainingTimeBeforeStopRecording = recordingTimeLeft / 1000;
	else
		remainingTimeBeforeStopRecording =
			recordingStopTimer->remainingTime() / 1000;

	int seconds = remainingTimeBeforeStopRecording % 60;
	int minutes = (remainingTimeBeforeStopRecording % 3600) / 60;
	int hours = remainingTimeBeforeStopRecording / 3600;

	QString text =
		QString::asprintf("%02d:%02d:%02d", hours, minutes, seconds);
	ui->recordStopTime->setText(text);
}

void NewOutputTimer::PauseRecordingTimer()
{
	if (!ui->pauseRecordTimer->isChecked())
		return;

	if (recordingStopTimer->isActive()) {
		recordingTimeLeft = recordingStopTimer->remainingTime();
		recordingStopTimer->stop();
	}
}

void NewOutputTimer::UnpauseRecordingTimer()
{
	if (!ui->pauseRecordTimer->isChecked())
		return;

	if (!recordingStopTimer->isActive())
		recordingStopTimer->start(recordingTimeLeft);
}

void NewOutputTimer::ShowHideDialog()
{
	if (!isVisible()) {
		setVisible(true);
		QTimer::singleShot(250, this, &NewOutputTimer::show);
	} else {
		setVisible(false);
		QTimer::singleShot(250, this, &NewOutputTimer::hide);
	}
}

void NewOutputTimer::EventStopStreaming()
{
	if (!stoppingStreamTimer) {
		blog(LOG_INFO, "Stopping stream due to NewOutputTimer timeout");
		obs_frontend_streaming_stop();
	}
}

void NewOutputTimer::EventStartRecording()
{
	if (!stoppingStartRecordingTimer) {
		blog(LOG_INFO,
		     "Starting recording due to NewOutputTimer timeout");
		obs_frontend_recording_start();
	}
}

void NewOutputTimer::EventStopRecording()
{
	if (!stoppingRecordingTimer) {
		blog(LOG_INFO,
		     "Stopping recording due to NewOutputTimer timeout");
		obs_frontend_recording_stop();
	}
}

static void SaveOutputTimer(obs_data_t *save_data, bool saving, void *)
{
	if (saving) {
		OBSDataAutoRelease obj = obs_data_create();

		obs_data_set_int(obj, "streamTimerHours",
				 ot->ui->streamingTimerHours->value());
		obs_data_set_int(obj, "streamTimerMinutes",
				 ot->ui->streamingTimerMinutes->value());
		obs_data_set_int(obj, "streamTimerSeconds",
				 ot->ui->streamingTimerSeconds->value());

		obs_data_set_int(obj, "startRecordTimerHours",
				 ot->ui->recordingStartTimerHours->value());
		obs_data_set_int(obj, "startRecordTimerMinutes",
				 ot->ui->recordingStartTimerMinutes->value());
		obs_data_set_int(obj, "startRecordTimerSeconds",
				 ot->ui->recordingStartTimerSeconds->value());

		obs_data_set_int(obj, "recordTimerHours",
				 ot->ui->recordingStopTimerHours->value());
		obs_data_set_int(obj, "recordTimerMinutes",
				 ot->ui->recordingStopTimerMinutes->value());
		obs_data_set_int(obj, "recordTimerSeconds",
				 ot->ui->recordingStopTimerSeconds->value());

		obs_data_set_bool(obj, "autoStartStreamTimer",
				  ot->ui->autoStartStreamTimer->isChecked());
		obs_data_set_bool(
			obj, "autoStartRecordTimer",
			ot->ui->autoStartRecordStopTimer->isChecked());

		obs_data_set_bool(obj, "pauseRecordTimer",
				  ot->ui->pauseRecordTimer->isChecked());

		obs_data_set_obj(save_data, "advanced-output-timer", obj);
	} else {
		OBSDataAutoRelease obj =
			obs_data_get_obj(save_data, "advanced-output-timer");

		if (!obj)
			obj = obs_data_create();

		ot->ui->streamingTimerHours->setValue(static_cast<int>(
			obs_data_get_int(obj, "streamTimerHours")));
		ot->ui->streamingTimerMinutes->setValue(static_cast<int>(
			obs_data_get_int(obj, "streamTimerMinutes")));
		ot->ui->streamingTimerSeconds->setValue(static_cast<int>(
			obs_data_get_int(obj, "streamTimerSeconds")));

		ot->ui->recordingStartTimerHours->setValue(static_cast<int>(
			obs_data_get_int(obj, "startRecordTimerHours")));
		ot->ui->recordingStartTimerMinutes->setValue(static_cast<int>(
			obs_data_get_int(obj, "startRecordTimerMinutes")));
		ot->ui->recordingStartTimerSeconds->setValue(static_cast<int>(
			obs_data_get_int(obj, "startRecordTimerSeconds")));

		ot->ui->recordingStopTimerHours->setValue(static_cast<int>(
			obs_data_get_int(obj, "recordTimerHours")));
		ot->ui->recordingStopTimerMinutes->setValue(static_cast<int>(
			obs_data_get_int(obj, "recordTimerMinutes")));
		ot->ui->recordingStopTimerSeconds->setValue(static_cast<int>(
			obs_data_get_int(obj, "recordTimerSeconds")));

		ot->ui->autoStartStreamTimer->setChecked(
			obs_data_get_bool(obj, "autoStartStreamTimer"));
		ot->ui->autoStartRecordStopTimer->setChecked(
			obs_data_get_bool(obj, "autoStartRecordTimer"));

		ot->ui->pauseRecordTimer->setChecked(
			obs_data_get_bool(obj, "pauseRecordTimer"));
	}
}

extern "C" void FreeOutputTimer() {}

static void OBSEvent(enum obs_frontend_event event, void *)
{
	if (event == OBS_FRONTEND_EVENT_EXIT) {
		obs_frontend_save();
		FreeOutputTimer();
	} else if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED) {
		ot->StreamTimerStart();
	} else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPING) {
		ot->StreamTimerStop();
	} else if (event == OBS_FRONTEND_EVENT_RECORDING_STARTED) {
		ot->ui->outputTimerRecordStart->setEnabled(false);
		ot->RecordTimerStart();
		ot->StartRecordTimerStop();
	} else if (event == OBS_FRONTEND_EVENT_RECORDING_STOPPING) {
		ot->RecordTimerStop();
		ot->ui->outputTimerRecordStart->setEnabled(true);
	} else if (event == OBS_FRONTEND_EVENT_RECORDING_PAUSED) {
		ot->PauseRecordingTimer();
	} else if (event == OBS_FRONTEND_EVENT_RECORDING_UNPAUSED) {
		ot->UnpauseRecordingTimer();
	}
}

extern "C" void InitOutputTimer()
{
	QAction *action = (QAction *)obs_frontend_add_tools_menu_qaction(
		obs_module_text("NewOutputTimer"));

	obs_frontend_push_ui_translation(obs_module_get_string);

	QMainWindow *window = (QMainWindow *)obs_frontend_get_main_window();

	ot = new NewOutputTimer(window);

	auto cb = []() {
		ot->ShowHideDialog();
	};

	obs_frontend_pop_ui_translation();

	obs_frontend_add_save_callback(SaveOutputTimer, nullptr);
	obs_frontend_add_event_callback(OBSEvent, nullptr);

	action->connect(action, &QAction::triggered, cb);
}
