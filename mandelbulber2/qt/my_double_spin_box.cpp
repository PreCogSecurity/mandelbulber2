/**
 * Mandelbulber v2, a 3D fractal generator       ,=#MKNmMMKmmßMNWy,
 *                                             ,B" ]L,,p%%%,,,§;, "K
 * Copyright (C) 2016 Mandelbulber Team        §R-==%w["'~5]m%=L.=~5N
 *                                        ,=mm=§M ]=4 yJKA"/-Nsaj  "Bw,==,,
 * This file is part of Mandelbulber.    §R.r= jw",M  Km .mM  FW ",§=ß., ,TN
 *                                     ,4R =%["w[N=7]J '"5=],""]]M,w,-; T=]M
 * Mandelbulber is free software:     §R.ß~-Q/M=,=5"v"]=Qf,'§"M= =,M.§ Rz]M"Kw
 * you can redistribute it and/or     §w "xDY.J ' -"m=====WeC=\ ""%""y=%"]"" §
 * modify it under the terms of the    "§M=M =D=4"N #"%==A%p M§ M6  R' #"=~.4M
 * GNU General Public License as        §W =, ][T"]C  §  § '§ e===~ U  !§[Z ]N
 * published by the                    4M",,Jm=,"=e~  §  §  j]]""N  BmM"py=ßM
 * Free Software Foundation,          ]§ T,M=& 'YmMMpM9MMM%=w=,,=MT]M m§;'§,
 * either version 3 of the License,    TWw [.j"5=~N[=§%=%W,T ]R,"=="Y[LFT ]N
 * or (at your option)                   TW=,-#"%=;[  =Q:["V""  ],,M.m == ]N
 * any later version.                      J§"mr"] ,=,," =="""J]= M"M"]==ß"
 *                                          §= "=C=4 §"eM "=B:m|4"]#F,§~
 * Mandelbulber is distributed in            "9w=,,]w em%wJ '"~" ,=,,ß"
 * the hope that it will be useful,                 . "K=  ,=RMMMßM"""
 * but WITHOUT ANY WARRANTY;                            .'''
 * without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with Mandelbulber. If not, see <http://www.gnu.org/licenses/>.
 *
 * ###########################################################################
 *
 * Authors: Krzysztof Marczak (buddhi1980@gmail.com)
 *
 * MyDoubleSpinBox class - promoted QDoubleSpinBox widget with context menu
 */

#include "my_double_spin_box.h"

#include <QLineEdit>

#include "../src/common_math.h"
#include "../src/system.hpp"
#include "frame_slider_popup.h"
#include "src/animation_flight.hpp"
#include "src/animation_keyframes.hpp"

MyDoubleSpinBox::MyDoubleSpinBox(QWidget *parent)
		: QDoubleSpinBox(parent), CommonMyWidgetWrapper(this)
{
	defaultValue = 0;
	slider = nullptr;
};

MyDoubleSpinBox::~MyDoubleSpinBox()
{
	if (slider) delete slider;
}

void MyDoubleSpinBox::paintEvent(QPaintEvent *event)
{
	QFont f = font();
	f.setBold(value() != GetDefault());
	setFont(f);
	QDoubleSpinBox::paintEvent(event);
}

double MyDoubleSpinBox::GetDefault()
{
	if (parameterContainer && !gotDefault)
	{
		QString type = GetType(objectName());
		if (type == QString("spinbox3") || type == QString("spinboxd3"))
		{
			char lastChar = (parameterName.at(parameterName.length() - 1)).toLatin1();
			QString nameVect = parameterName.left(parameterName.length() - 2);
			CVector3 val = parameterContainer->GetDefault<CVector3>(nameVect);
			defaultValue = val.itemByName(lastChar);
			gotDefault = true;
			setToolTipText();
		}
		else if (type == QString("spinbox4") || type == QString("spinboxd4"))
		{
			char lastChar = (parameterName.at(parameterName.length() - 1)).toLatin1();
			QString nameVect = parameterName.left(parameterName.length() - 2);
			CVector4 val = parameterContainer->GetDefault<CVector4>(nameVect);
			defaultValue = val.itemByName(lastChar);
			gotDefault = true;
			setToolTipText();
		}
		else
		{
			defaultValue = parameterContainer->GetDefault<double>(parameterName);
			gotDefault = true;
			setToolTipText();
		}
	}
	return defaultValue;
}

void MyDoubleSpinBox::resetToDefault()
{
	setValue(defaultValue);
	emit valueChanged(defaultValue);
}

QString MyDoubleSpinBox::getDefaultAsString()
{
	return QString("%L1").arg(defaultValue, 0, 'g', 16);
}

QString MyDoubleSpinBox::getFullParameterName()
{
	QString parName = parameterName;
	QString type = GetType(objectName());
	if (type == QString("spinbox3") || type == QString("spinboxd3") || type == QString("spinbox4")
			|| type == QString("spinboxd4"))
		parName = parameterName.left(parameterName.length() - 2);
	return parName;
}

void MyDoubleSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
	CommonMyWidgetWrapper::contextMenuEvent(event, lineEdit()->createStandardContextMenu());
}

void MyDoubleSpinBox::focusInEvent(QFocusEvent *event)
{
	QDoubleSpinBox::focusInEvent(event);

	QString type = GetType(objectName());
	if (type != "text")
	{
		if (!slider)
		{
			QWidget *topWidget = this->window();
			slider = new cFrameSiderPopup(topWidget);
			slider->setFocusPolicy(Qt::NoFocus);
			slider->hide();
		}

		QWidget *topWidget = this->window();
		QPoint windowPoint = this->mapTo(topWidget, QPoint());
		int width = this->width();
		int hOffset = this->height();
		slider->adjustSize();
		slider->setFixedWidth(width);
		slider->move(windowPoint.x(), windowPoint.y() + hOffset);
		slider->show();

		connect(slider, SIGNAL(timerTrigger()), this, SLOT(slotSliderTimerUpdateValue()));
		connect(slider, SIGNAL(resetPressed()), this, SLOT(slotResetToDefault()));
		connect(slider, SIGNAL(zeroPressed()), this, SLOT(slotZerovalue()));
		connect(slider, SIGNAL(halfPressed()), this, SLOT(slotHalfValue()));
		connect(slider, SIGNAL(doublePressed()), this, SLOT(slotDoublevalue()));
		connect(slider, SIGNAL(intPressed()), this, SLOT(slotRoundValue()));
	}
}

void MyDoubleSpinBox::focusOutEvent(QFocusEvent *event)
{
	QDoubleSpinBox::focusOutEvent(event);

	if (slider)
	{
		slider->disconnect();
		slider->hide();
	}
}

void MyDoubleSpinBox::slotSliderTimerUpdateValue()
{
	const double val = value();
	int iDiff = slider->value() - 500;
	if (iDiff != 0)
	{
		double sign = (iDiff > 0) ? 1.0 : -1.0;
		double digits = decimals();
		double dDiff = abs(iDiff) / (600.0 / digits) - digits;
		double change = pow(10.0, dDiff) * sign;
		setValue(val + change);
		emit editingFinished();
	}
}

void MyDoubleSpinBox::slotResetToDefault()
{
	resetToDefault();
}

void MyDoubleSpinBox::slotZerovalue()
{
	setValue(0.0);
}

void MyDoubleSpinBox::slotDoublevalue()
{
	const double val = value();
	setValue(val * 2.0);
}

void MyDoubleSpinBox::slotHalfValue()
{
	const double val = value();
	setValue(val * 0.5);
}

void MyDoubleSpinBox::slotRoundValue()
{
	const double val = value();
	setValue(MagicRound(val, 0.05));
}
