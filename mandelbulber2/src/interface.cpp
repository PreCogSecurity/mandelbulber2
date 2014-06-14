/*
 * interface.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: krzysztof
 */


#include "interface.hpp"
#include "system.hpp"
#include <QTextStream>
#include <QtUiTools/QtUiTools>
#include "fractal_list.hpp"
#include <qpainter.h>
#include "render_job.hpp"
#include "calculate_distance.hpp"
#include "camera_target.hpp"

cInterface *mainInterface;

//constructor of interface (loading of ui files)
cInterface::cInterface()
{
	mainWindow = NULL;
	application = NULL;
	qimage = NULL;
	renderedImage = NULL;
	mainImage = NULL;
	progressBar = NULL;
	stopRequest = false;
	repeatRequest = false;
}

void cInterface::ShowUi(void)
{
	WriteLog("Prepare RenderWindow class");
	mainWindow = new RenderWindow;

	WriteLog("Restoring window geometry");
	mainWindow->restoreGeometry(mainWindow->settings.value("mainWindowGeometry").toByteArray());

	WriteLog("Restoring window state");
	mainWindow->restoreState(mainWindow->settings.value("mainWindowState").toByteArray());

	WriteLog("mainWindow->show()");
	mainWindow->show();

	WriteLog("Prepare RenderedImage class");
	renderedImage = new RenderedImage(mainWindow);
	mainWindow->ui->scrollAreaLayoutRenderedImage->addWidget(renderedImage);

	progressBar = new QProgressBar(mainWindow->ui->statusbar);
	mainWindow->ui->statusbar->addPermanentWidget(progressBar);

	renderedImage->show();

	//loading default ui for all fractal components
	QString uiFilename = systemData.sharedDir + QDir::separator() + "qt" + QDir::separator() + "fractal_mandelbulb.ui";
	InitializeFractalUi(uiFilename);

	WriteLog("ConnectSignals()");
	ConnectSignals();
}

void cInterface::ConnectSignals(void)
{
	QApplication::connect(mainWindow->ui->pushButton_render, SIGNAL(clicked()), mainWindow, SLOT(slotStartRender()));
	QApplication::connect(mainWindow->ui->pushButton_stop, SIGNAL(clicked()), mainWindow, SLOT(slotStopRender()));
	QApplication::connect(mainWindow->ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
	QApplication::connect(mainWindow->ui->actionSave_docks_positions, SIGNAL(triggered()), mainWindow, SLOT(slotMenuSaveDocksPositions()));

	QApplication::connect(mainWindow->ui->comboBox_formula_1, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedFractalCombo(int)));
	QApplication::connect(mainWindow->ui->comboBox_formula_2, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedFractalCombo(int)));
	QApplication::connect(mainWindow->ui->comboBox_formula_3, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedFractalCombo(int)));
	QApplication::connect(mainWindow->ui->comboBox_formula_4, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedFractalCombo(int)));

	QApplication::connect(mainWindow->ui->scrollAreaForImage, SIGNAL(resized(int, int)), mainWindow, SLOT(slotImageScrolledAreaResized(int, int)));
	QApplication::connect(mainWindow->ui->comboBox_image_preview_scale, SIGNAL(currentIndexChanged(int)), mainWindow, SLOT(slotChangedImageScale(int)));

	// ------------ camera manipulation -----------
	QApplication::connect(mainWindow->ui->bu_move_up, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_down, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_left, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_right, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_forward, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));
	QApplication::connect(mainWindow->ui->bu_move_backward, SIGNAL(clicked()), mainWindow, SLOT(slotCameraMove()));

	QApplication::connect(mainWindow->ui->bu_rotate_up, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_down, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_left, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_right, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_roll_left, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));
	QApplication::connect(mainWindow->ui->bu_rotate_roll_right, SIGNAL(clicked()), mainWindow, SLOT(slotCameraRotation()));

	QApplication::connect(mainWindow->ui->vect3_camera_x, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_y, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_z, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_target_x, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_target_y, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_target_z, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraOrTargetEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_rotation_x, SIGNAL(editingFinished()), mainWindow, SLOT(slotRotationEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_rotation_y, SIGNAL(editingFinished()), mainWindow, SLOT(slotRotationEdited()));
	QApplication::connect(mainWindow->ui->vect3_camera_rotation_z, SIGNAL(editingFinished()), mainWindow, SLOT(slotRotationEdited()));
	QApplication::connect(mainWindow->ui->logedit_camera_distance_to_target, SIGNAL(editingFinished()), mainWindow, SLOT(slotCameraDistanceEdited()));
	QApplication::connect(mainWindow->ui->logslider_camera_distance_to_target, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotCameraDistanceSlider(int)));

	//------------------------------------------------

	ConnectSignalsForSlidersInWindow(mainWindow);
	MakeColorButtonsInWindow(mainWindow);
}

//Reading ad writing parameters from/to ui to/from parameters container
void cInterface::SynchronizeInterface(parameters::container *par, parameters::container *parFractal, enumReadWrite mode)
{
	SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_effects, par, mode);
	SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_image_adjustments, par, mode);
	SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_navigation, par, mode);
	SynchronizeInterfaceWindow(mainWindow->ui->dockWidget_rendering_engine, par, mode);
	SynchronizeInterfaceWindow(mainWindow->ui->page_fractal_common, par, mode);
	SynchronizeInterfaceWindow(mainWindow->ui->page_fractal_hybrid, par, mode);
	SynchronizeInterfaceWindow(mainWindow->ui->centralwidget, par, mode);

	SynchronizeInterfaceWindow(mainWindow->ui->tab_fractal_formula_1, &parFractal[0], mode);
	SynchronizeInterfaceWindow(mainWindow->ui->tab_fractal_formula_2, &parFractal[1], mode);
	SynchronizeInterfaceWindow(mainWindow->ui->tab_fractal_formula_3, &parFractal[2], mode);
	SynchronizeInterfaceWindow(mainWindow->ui->tab_fractal_formula_4, &parFractal[3], mode);

}

//Reading ad writing parameters from/to selected widget to/from parameters container
void cInterface::SynchronizeInterfaceWindow(QWidget *window, parameters::container *par, enumReadWrite mode)
{
	QTextStream out(stdout);
	WriteLog("SynchronizeInterfaceWindow() started");

	//----------- QLineEdit -------------------
	{
		WriteLog("SynchronizeInterfaceWindow() QLineEdit");
		QList<QLineEdit *> widgetListLineEdit = window->findChildren<QLineEdit *>();
		QList<QLineEdit *>::iterator it;

		for (it = widgetListLineEdit.begin(); it != widgetListLineEdit.end(); ++it)
		{
			//out << "QLineEdit:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;

			QString name = (*it)->objectName();
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QLineEdit"))
			{
				QLineEdit *lineEdit = *it;
				QString text = lineEdit->text();
				//out << name << " - text: " << text << endl;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);
				//out << name << " - type: " << type << endl;

				//----- get vectors ------------
				if (type == QString("vect3"))
				{
					char lastChar = (parameterName.at(parameterName.length() - 1)).toLatin1();
					QString nameVect = parameterName.left(parameterName.length() - 2);

					if (mode == read)
					{
						double value = lineEdit->text().toDouble();
						//out << nameVect << " - " << lastChar << " axis = " << value << endl;
						CVector3 vect = par->Get<CVector3>(nameVect);

						switch (lastChar)
						{
							case 'x':
								vect.x = value;
								break;

							case 'y':
								vect.y = value;
								break;

							case 'z':
								vect.z = value;
								break;

							default:
								qWarning() << "cInterface::SynchronizeInterfaceWindow(): edit field " << nameVect << " has wrong axis name (is " << lastChar << ")" << endl;
								break;
						}
						par->Set(nameVect, vect);
					}
					else if (mode == write)
					{
						CVector3 vect = par->Get<CVector3>(nameVect);
						QString qtext;

						switch (lastChar)
						{
							case 'x':
								qtext = QString::number(vect.x, 'g', 20);
								break;

							case 'y':
								qtext = QString::number(vect.y, 'g', 20);
								break;

							case 'z':
								qtext = QString::number(vect.z, 'g', 20);
								break;

							default:
								qWarning() << "cInterface::SynchronizeInterfaceWindow(): edit field " << nameVect << " has wrong axis name (is " << lastChar << ")" << endl;
								break;
						}
						lineEdit->setText(qtext);
						lineEdit->setCursorPosition(0);
					}
				}

				//---------- get double scalars --------
				else if (type == QString("edit") || type == QString("logedit"))
				{
					if (mode == read)
					{
						double value = lineEdit->text().toDouble();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						double value = par->Get<double>(parameterName);
						lineEdit->setText(QString::number(value, 'g', 20));
						lineEdit->setCursorPosition(0);
					}
				}

				//----------- get texts ------------
				else if (type == QString("text"))
				{
					if (mode == read)
					{
						QString text = lineEdit->text();
						par->Set(parameterName, text);
					}
					else if (mode == write)
					{
						QString text = par->Get<QString>(parameterName);
						lineEdit->setText(text);
					}
				}
			}
		} //end foreach
	}

	//------------ Double spin-box --------------
	{
		WriteLog("SynchronizeInterfaceWindow() QDoubleSpinBox");
		QList<QDoubleSpinBox *> widgetListDoubleSpinBox = window->findChildren<QDoubleSpinBox*>();
		QList<QDoubleSpinBox *>::iterator it;
		for (it = widgetListDoubleSpinBox.begin(); it != widgetListDoubleSpinBox.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QDoubleSpinBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QDoubleSpinBox"))
			{
				QDoubleSpinBox *spinbox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("spinbox"))
				{
					if (mode == read)
					{
						double value = spinbox->value();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						double value = par->Get<double>(parameterName);
						spinbox->setValue(value);
					}
				}
			}
		}
	}

	//------------ integer spin-box --------------
	{
		WriteLog("SynchronizeInterfaceWindow() QSpinBox");
		QList<QSpinBox *> widgetListDoubleSpinBox = window->findChildren<QSpinBox*>();
		QList<QSpinBox *>::iterator it;
		for (it = widgetListDoubleSpinBox.begin(); it != widgetListDoubleSpinBox.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QDoubleSpinBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QSpinBox"))
			{
				QSpinBox *spinbox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("spinboxInt"))
				{
					if (mode == read)
					{
						int value = spinbox->value();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						int value = par->Get<int>(parameterName);
						spinbox->setValue(value);
					}
				}
			}
		}
	}

	//checkboxes
	{
		WriteLog("SynchronizeInterfaceWindow() QCheckBox");
		QList<QCheckBox *> widgetListDoubleSpinBox = window->findChildren<QCheckBox*>();
		QList<QCheckBox *>::iterator it;
		for (it = widgetListDoubleSpinBox.begin(); it != widgetListDoubleSpinBox.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QCheckBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QCheckBox"))
			{
				QCheckBox *checkbox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("checkBox"))
				{
					if (mode == read)
					{
						bool value = checkbox->isChecked();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						bool value = par->Get<bool>(parameterName);
						checkbox->setChecked(value);
					}
				}
			}
		}
	}

	//groupsBox with checkbox
	{
		WriteLog("SynchronizeInterfaceWindow() QGroupBox");
		QList<QGroupBox *> widgetListDoubleSpinBox = window->findChildren<QGroupBox*>();
		QList<QGroupBox *>::iterator it;
		for (it = widgetListDoubleSpinBox.begin(); it != widgetListDoubleSpinBox.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QGroupBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QGroupBox"))
			{
				QGroupBox *groupbox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("groupCheck"))
				{
					if (mode == read)
					{
						bool value = groupbox->isChecked();
						par->Set(parameterName, value);
					}
					else if (mode == write)
					{
						bool value = par->Get<bool>(parameterName);
						groupbox->setChecked(value);
					}
				}
			}
		}
	}

	//---------- color buttons -----------
	{
		WriteLog("SynchronizeInterfaceWindow() colorButtons");
		QList<QPushButton *> widgetListPushButton = window->findChildren<QPushButton*>();
		QList<QPushButton *>::iterator it;
		for (it = widgetListPushButton.begin(); it != widgetListPushButton.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QDoubleSpinBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QPushButton"))
			{
				QPushButton *colorButton = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("colorButton"))
				{
					if (mode == read)
					{
						sRGB color;
						color.R = colorButton->property("selectedColor_r").toInt();
						color.G = colorButton->property("selectedColor_g").toInt();
						color.B = colorButton->property("selectedColor_b").toInt();
						par->Set(parameterName, color);
					}
					else if (mode == write)
					{
						sRGB color = par->Get<sRGB>(parameterName);
						QColor qcolor(color.R / 256, color.G / 256, color.B / 256);
						MakeIconForButton(qcolor, colorButton);
						colorButton->setProperty("selectedColor_r", color.R);
						colorButton->setProperty("selectedColor_g", color.G);
						colorButton->setProperty("selectedColor_b", color.B);
					}
				}
			}
		}
	}
	WriteLog("SynchronizeInterfaceWindow() finished");

	//combo boxes
	{
		WriteLog("SynchronizeInterfaceWindow() combo boxes");
		QList<QComboBox *> widgetListPushButton = window->findChildren<QComboBox*>();
		QList<QComboBox *>::iterator it;
		for (it = widgetListPushButton.begin(); it != widgetListPushButton.end(); ++it)
		{
			QString name = (*it)->objectName();
			//out << "QComboBox:" << (*it)->objectName() << " Type:" << (*it)->metaObject()->className() << endl;
			if (name.length() > 1 && (*it)->metaObject()->className() == QString("QComboBox"))
			{
				QComboBox *comboBox = *it;

				QString type, parameterName;
				GetNameAndType(name, &parameterName, &type);

				if (type == QString("comboBox"))
				{
					if (mode == read)
					{
						int selection = comboBox->currentIndex();
						par->Set(parameterName, selection);
					}
					else if (mode == write)
					{
						int selection = par->Get<int>(parameterName);
						comboBox->setCurrentIndex(selection);
					}
				}
			}
		}
	}

}

//automatic setting of event slots for all sliders
void cInterface::ConnectSignalsForSlidersInWindow(QWidget *window)
{
	WriteLog("ConnectSignalsForSlidersInWindow() started");

	QList<QSlider *> widgetList = window->findChildren<QSlider *>();
	QList<QSlider *>::iterator it;
	for (it = widgetList.begin(); it != widgetList.end(); ++it)
	{
		QString name = (*it)->objectName();
		if (name.length() > 1 && (*it)->metaObject()->className() == QString("QSlider"))
		{
			const QSlider *slider = *it;

			QString type, parameterName;
			GetNameAndType(name, &parameterName, &type);

			if (type == QString("slider"))
			{
				QApplication::connect(slider, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotSliderMoved(int)));

				QString spinBoxName = QString("spinbox_") + parameterName;
				QDoubleSpinBox *spinBox = slider->parent()->findChild<QDoubleSpinBox*>(spinBoxName);
				if (spinBox)
				{
					QApplication::connect(spinBox, SIGNAL(valueChanged(double)), mainWindow, SLOT(slotDoubleSpinBoxChanged(double)));
				}
				else
				{
					qWarning() << "ConnectSignalsForSlidersInWindow() error: spinbox " << spinBoxName << " doesn't exists" << endl;
				}
			}
			if (type == QString("logslider"))
			{
				QApplication::connect(slider, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotLogSliderMoved(int)));

				QString editFieldName = QString("logedit_") + parameterName;
				QLineEdit *lineEdit = slider->parent()->findChild<QLineEdit*>(editFieldName);
				if (lineEdit)
				{
					QApplication::connect(lineEdit, SIGNAL(textChanged(const QString&)), mainWindow, SLOT(slotLogLineEditChanged(const QString&)));
				}
				else
				{
					qWarning() << "ConnectSignalsForSlidersInWindow() error: lineEdit " << editFieldName << " doesn't exists" << endl;
				}
			}
			if (type == QString("sliderInt"))
			{
				QApplication::connect(slider, SIGNAL(sliderMoved(int)), mainWindow, SLOT(slotIntSliderMoved(int)));

				QString spinboxName = QString("spinboxInt_") + parameterName;
				QSpinBox *spinbox = slider->parent()->findChild<QSpinBox*>(spinboxName);
				if (spinbox)
				{
					QApplication::connect(spinbox, SIGNAL(valueChanged(int)), mainWindow, SLOT(slotIntSpinBoxChanged(int)));
				}
				else
				{
					qWarning() << "ConnectSignalsForSlidersInWindow() error: spinboxInt " << spinboxName << " doesn't exists" << endl;
				}
			}
		}
	}
	WriteLog("ConnectSignalsForSlidersInWindow() finished");
}

//automatic setting of event slots for all colorButtons and preparation of buttons
void cInterface::MakeColorButtonsInWindow(QWidget *window)
{
	WriteLog("MakeColorButtonsInWindow() started");

	QList<QPushButton *> widgetList = window->findChildren<QPushButton *>();
	QList<QPushButton *>::iterator it;
	for(it = widgetList.begin(); it != widgetList.end(); ++it)
	{
		QString name = (*it)->objectName();
		if(name.length() > 1 && (*it)->metaObject()->className() == QString("QPushButton"))
		{
			QPushButton *pushButton = (*it);
			QString type, parameterName;
			GetNameAndType(name, &parameterName, &type);

			if(type == QString("colorButton"))
			{
				pushButton->setText("");
				QColor color(255,255,255);
				MakeIconForButton(color, pushButton);
				QApplication::connect(pushButton, SIGNAL(clicked()), mainWindow, SLOT(slotPresedOnColorButton()));
			}
		}
	}
	WriteLog("MakeColorButtonsInWindow() finished");
}

//extract name and type string from widget name
void cInterface::GetNameAndType(QString name, QString *parameterName, QString *type)
{
	size_t firstDashPosition = name.indexOf("_");
	*type = name.left(firstDashPosition);
	*parameterName = name.mid(firstDashPosition + 1);
}

//initialize ui for hybrid fractal components
void cInterface::InitializeFractalUi(QString &uiFileName)
{
	QUiLoader loader;
	QFile uiFile(uiFileName);

	if(uiFile.exists())
	{
		uiFile.open(QFile::ReadOnly);
		mainWindow->fractalWidgets[0] = loader.load(&uiFile);
		mainWindow->ui->verticalLayout_fractal_1->addWidget(mainWindow->fractalWidgets[0]);
		//mainWindow->ui->verticalLayout_fractal_1->addStretch(1);
		uiFile.seek(0);
		mainWindow->fractalWidgets[1] = loader.load(&uiFile);
		mainWindow->ui->verticalLayout_fractal_2->addWidget(mainWindow->fractalWidgets[1]);
		//mainWindow->ui->verticalLayout_fractal_2->addStretch(1);
		uiFile.seek(0);
		mainWindow->fractalWidgets[2] = loader.load(&uiFile);
		mainWindow->ui->verticalLayout_fractal_3->addWidget(mainWindow->fractalWidgets[2]);
		//mainWindow->ui->verticalLayout_fractal_3->addStretch(1);
		uiFile.seek(0);
		mainWindow->fractalWidgets[3] = loader.load(&uiFile);
		mainWindow->ui->verticalLayout_fractal_4->addWidget(mainWindow->fractalWidgets[3]);
		//mainWindow->ui->verticalLayout_fractal_4->addStretch(1);
		uiFile.close();

		for (int i = 0; i < 4; i++)
			mainWindow->fractalWidgets[i]->show();

		QStringList fractalNames;
		for (int i = 0; i < fractalList.size(); i++)
		{
			QString name = fractalList[i].nameInComboBox;
			fractalNames.append(name);
		}

		mainWindow->ui->comboBox_formula_1->clear();
		mainWindow->ui->comboBox_formula_1->addItems(fractalNames);
		mainWindow->ui->comboBox_formula_2->clear();
		mainWindow->ui->comboBox_formula_2->addItems(fractalNames);
		mainWindow->ui->comboBox_formula_3->clear();
		mainWindow->ui->comboBox_formula_3->addItems(fractalNames);
		mainWindow->ui->comboBox_formula_4->clear();
		mainWindow->ui->comboBox_formula_4->addItems(fractalNames);
	}
	else
	{
		qCritical() << "Can't open file " << uiFileName << " Fractal ui files can't be loaded";
	}
}

void cInterface::StatusText(QString &text, QString &progressText, double progress)
{
	mainWindow->ui->statusbar->showMessage(text, 0);
	mainInterface->progressBar->setValue(progress * 100.0);
	mainInterface->progressBar->setTextVisible(true);
	mainInterface->progressBar->setFormat(progressText);
}

double cInterface::ImageScaleComboSelection2Double(int index)
{
	double scales[] = {0.0, 4.0, 2.0, 1.0, 0.5, 0.25, 0.1};
	if(index < 7)
	{
		return scales[index];
	}
	else
	{
		qCritical() << "Wrong image scale";
		return -1.0;
	}
}

double cInterface::CalcMainImageScale(double scale, int previewWidth, int previewHeight, cImage *image)
{
	double scaleOut;
	if(scale == 0.0)
	{
		double scale1 = (double)previewHeight / image->GetHeight();
		double scale2 = (double)previewWidth / image->GetWidth();
		scaleOut = min(scale1, scale2);
	}
	else
	{
		scaleOut = scale;
	}
	return scaleOut;
}

void cInterface::StartRender(void)
{
	if(mainImage->IsUsed())
	{
		stopRequest = true;
		repeatRequest = true;
	}
	else
	{
	  do
	  {
			repeatRequest = false;
	  	SynchronizeInterface(gPar, gParFractal, cInterface::read);
			cRenderJob *renderJob = new cRenderJob(gPar, gParFractal, mainImage, renderedImage);
			renderJob->Init(cRenderJob::still);
			renderJob->Execute();

			delete renderJob;
	  }
		while(repeatRequest);
	}
}

void cInterface::MoveCamera(QString buttonName)
{
	//get data from interface
	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::read);
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);

	//get direction vector
	CVector3 direction;
	if(buttonName == "bu_move_left")
		direction = cameraTarget.GetRightVector() * (-1.0);
	else if(buttonName == "bu_move_right")
		direction = cameraTarget.GetRightVector() * ( 1.0);
	else if(buttonName == "bu_move_up")
		direction = cameraTarget.GetTopVector() * ( 1.0);
	else if(buttonName == "bu_move_down")
		direction = cameraTarget.GetTopVector() * (-1.0);
	else if(buttonName == "bu_move_forward")
		direction = cameraTarget.GetForwardVector() * (1.0);
	else if(buttonName == "bu_move_backward")
		direction = cameraTarget.GetForwardVector() * (-1.0);

	enumCameraMovementStepMode stepMode = (enumCameraMovementStepMode)gPar->Get<int>("camera_absolute_distance_mode");
	enumCameraMovementMode movementMode = (enumCameraMovementMode)gPar->Get<int>("camera_movement_mode");

	//movement step
	double step;
	if(stepMode == absolute)
	{
		step = gPar->Get<double>("camera_movenent_step");
	}
	else
	{
		double relativeStep = gPar->Get<double>("camera_movenent_step");

		cParamRender *params = new cParamRender(gPar);
		cFourFractals *fourFractals = new cFourFractals(gParFractal);
		sDistanceIn in(camera, 0, false);
		if(movementMode == moveTarget)
			in.point = target;

		sDistanceOut out;
		double distance = CalculateDistance(*params, *fourFractals, in, &out);
		delete params;
		delete fourFractals;

		step = relativeStep * distance;
	}

	//movement
	if(movementMode == moveCamera)
		camera += direction * step;
	else if(movementMode == moveTarget)
		target += direction * step;
	else if (movementMode == fixedDistance)
	{
		camera += direction * step;
		target += direction * step;
	}

	//put data to interface
	gPar->Set("camera", camera);
	gPar->Set("target", target);

	//recalculation of camera-target
	cCameraTarget::enumRotationMode rollMode = 	(cCameraTarget::enumRotationMode)gPar->Get<int>("camera_straight_rotation");
	if(movementMode == moveCamera)
		cameraTarget.SetCamera(camera, rollMode);
	else if(movementMode == moveTarget)
		cameraTarget.SetTarget(target, rollMode);
	else if (movementMode == fixedDistance)
		cameraTarget.SetCameraTargetTop(camera, target, topVector);

	topVector = cameraTarget.GetTopVector();
	gPar->Set("camera_top", topVector);
	CVector3 rotation = cameraTarget.GetRotation();
	gPar->Set("camera_rotation", rotation * (180.0 / M_PI));
	double dist = cameraTarget.GetDistance();
	gPar->Set("camera_distance_to_target", dist);

	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::write);

	StartRender();
}

void cInterface::CameraOrTargetEdited(void)
{
	//get data from interface before synchronization
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);

	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::read);
	camera = gPar->Get<CVector3>("camera");
	target = gPar->Get<CVector3>("target");

	//recalculation of camera-target
	cCameraTarget::enumRotationMode rollMode = 	(cCameraTarget::enumRotationMode)gPar->Get<int>("camera_straight_rotation");
	cameraTarget.SetCamera(camera, rollMode);
	cameraTarget.SetTarget(target, rollMode);

	topVector = cameraTarget.GetTopVector();
	gPar->Set("camera_top", topVector);
	CVector3 rotation = cameraTarget.GetRotation();
	gPar->Set("camera_rotation", rotation * (180.0 / M_PI));
	double dist = cameraTarget.GetDistance();
	gPar->Set("camera_distance_to_target", dist);

	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::write);

}

void cInterface::RotateCamera(QString buttonName)
{
	//get data from interface
	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::read);
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);
	double distance = cameraTarget.GetDistance();

	enumCameraRotationMode rotationMode = (enumCameraRotationMode)gPar->Get<int>("camera_rotation_mode");
	cCameraTarget::enumRotationMode rollMode = 	(cCameraTarget::enumRotationMode)gPar->Get<int>("camera_straight_rotation");

	CVector3 rotationAxis;
	if(rollMode == cCameraTarget::constantRoll)
	{
		if(buttonName == "bu_rotate_left")
			rotationAxis = CVector3(0.0, 0.0, 1.0);
		else if(buttonName == "bu_rotate_right")
			rotationAxis = CVector3(0.0, 0.0, -1.0);
		else if(buttonName == "bu_rotate_up")
			rotationAxis = CVector3(1.0, 0.0, 0.0);
		else if(buttonName == "bu_rotate_down")
			rotationAxis = CVector3(-1.0, 0.0, 0.0);
		else if(buttonName == "bu_rotate_roll_left")
			rotationAxis = CVector3(0.0, -1.0, 0.0);
		else if(buttonName == "bu_rotate_roll_right")
			rotationAxis = CVector3(0.0, 1.0, 0.0);
	}
	else
	{
		if(buttonName == "bu_rotate_left")
			rotationAxis = cameraTarget.GetTopVector() * ( 1.0);
		else if(buttonName == "bu_rotate_right")
			rotationAxis = cameraTarget.GetTopVector() * (-1.0);
		else if(buttonName == "bu_rotate_up")
			rotationAxis = cameraTarget.GetRightVector() * (1.0);
		else if(buttonName == "bu_rotate_down")
			rotationAxis = cameraTarget.GetRightVector() * (-1.0);
		else if(buttonName == "bu_rotate_roll_left")
			rotationAxis = cameraTarget.GetForwardVector() * (-1.0);
		else if(buttonName == "bu_rotate_roll_right")
			rotationAxis = cameraTarget.GetForwardVector() * (1.0);
	}

	if(rotationMode == rotateAroundTarget)
		rotationAxis *= -1.0;

	//rotation of vectors
	CVector3 forwardVector = cameraTarget.GetForwardVector();
	double rotationStep = gPar->Get<double>("camera_rotation_step") * (M_PI / 180.0);
	forwardVector = forwardVector.RotateAroundVectorByAngle(rotationAxis, rotationStep);
	topVector = topVector.RotateAroundVectorByAngle(rotationAxis, rotationStep);

	if(rotationMode == rotateCamera)
	{
		target = camera + forwardVector * distance;
	}
	else
	{
		camera = target - forwardVector * distance;
	}

	//recalculation of camera-target
	cameraTarget.SetCameraTargetTop(camera, target, topVector);

	gPar->Set("camera", camera);
	gPar->Set("target", target);
	gPar->Set("camera_top", topVector);
	CVector3 rotation = cameraTarget.GetRotation();
	gPar->Set("camera_rotation", rotation * (180.0 / M_PI));
	double dist = cameraTarget.GetDistance();
	gPar->Set("camera_distance_to_target", dist);

	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::write);

	StartRender();
}

void cInterface::RotationEdited(void)
{
	//get data from interface before synchronization
	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::read);
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);
	double distance = cameraTarget.GetDistance();
	CVector3 rotation = gPar->Get<CVector3>("camera_rotation") * (M_PI / 180.0);

	enumCameraRotationMode rotationMode = (enumCameraRotationMode)gPar->Get<int>("camera_rotation_mode");

	CVector3 forwardVector(0.0, 1.0, 0.0);
	forwardVector = forwardVector.RotateAroundVectorByAngle(CVector3(0.0, 1.0, 0.0), rotation.z);
	forwardVector = forwardVector.RotateAroundVectorByAngle(CVector3(1.0, 0.0, 0.0), rotation.y);
	forwardVector = forwardVector.RotateAroundVectorByAngle(CVector3(0.0, 0.0, 1.0), rotation.x);

	if(rotationMode == rotateCamera)
	{
		target = camera + forwardVector * distance;
	}
	else
	{
		camera = target - forwardVector * distance;
	}
	cameraTarget.SetCameraTargetRotation(camera, target, rotation.z);
	gPar->Set("camera", camera);
	gPar->Set("target", target);
	gPar->Set("camera_top", cameraTarget.GetTopVector());
	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::write);
}

void cInterface::CameraDistanceEdited()
{
	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::read);
	CVector3 camera = gPar->Get<CVector3>("camera");
	CVector3 target = gPar->Get<CVector3>("target");
	CVector3 topVector = gPar->Get<CVector3>("camera_top");
	cCameraTarget cameraTarget(camera, target, topVector);
	CVector3 forwardVector = cameraTarget.GetForwardVector();

	double distance = gPar->Get<double>("camera_distance_to_target");

	enumCameraMovementMode movementMode = (enumCameraMovementMode)gPar->Get<int>("camera_movement_mode");
	if(movementMode == moveTarget)
	{
		target = camera + forwardVector * distance;
	}
	else
	{
		camera = target - forwardVector * distance;
	}

	cameraTarget.SetCameraTargetTop(camera, target, topVector);
	gPar->Set("camera", camera);
	gPar->Set("target", target);
	gPar->Set("camera_top", cameraTarget.GetTopVector());
	mainInterface->SynchronizeInterface(gPar, gParFractal, cInterface::write);

}


//function to create icons with actual color in ColorButtons
void MakeIconForButton(QColor &color, QPushButton *pushbutton)
{
	const int w = 40;
	const int h = 15;
	QPixmap pix(w,h);
	QPainter painter(&pix);
	painter.fillRect(QRect(0,0,w,h), color);
	painter.drawRect(0,0,w-1,h-1);
	QIcon icon(pix);
	pushbutton->setIcon(icon);
	pushbutton->setIconSize(QSize(w,h));
}



