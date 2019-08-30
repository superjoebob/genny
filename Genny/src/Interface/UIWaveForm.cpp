#include "UIWaveForm.h"
#include "UIBitmap.h"
#include "../resource.h"
#include "IndexBaron.h"
#include "GennyVST.h"
#include "UIImage.h"
#include "UIInstrumentsPanel.h"

UIWaveForm::UIWaveForm(CPoint point, UIInstrument* owner):
	CControl(CRect(point.x, point.y, point.x + 154, point.y + 34), owner),
	GennyInterfaceObject(owner),
	_xpos(point.x),
	_ypos(point.y)
{
	CFrame* frame = owner->getFrame();
	frame->addView(this);
	reconnect();
}

UIWaveForm::~UIWaveForm(void)
{

}

void UIWaveForm::setValue(float val)
{
	CControl::setValue(val);
}
void UIWaveForm::valueChanged (CControl* control)
{

	setDirty(true);
}

void UIWaveForm::draw (CDrawContext* pContext)
{
	if(isVisible())
	{
		WaveData* wave = ((GennyPatch*)getCurrentPatch())->InstrumentDef.Drumset.getDrum(36 + ((GennyPatch*)getCurrentPatch())->InstrumentDef.SelectedDrum);
		if(wave != nullptr)
		{
			float width = 152;
			float height = 17;

			pContext->setFrameColor(CColor(33, 53, 29, 255));
			pContext->setLineStyle (kLineSolid);
			pContext->setLineWidth(2);
			pContext->moveTo(CPoint(_xpos, _ypos + 17));
			pContext->lineTo(CPoint(_xpos + 154, _ypos + 17)); 

			for(int i  = 0; i < 77; i++)
			{
				int dataIndex = (wave->size / 77) * i;
				if(dataIndex < wave->size - 128)
				{
					char largest = 0;
					for(int samp = 0; samp < 128; samp+= 2)
					{
						char val = abs(wave->audioData[dataIndex] - 127);
						if(val > largest)
							largest = val;
					}

					float average = (largest / 127.0f);
					average *= 12.0f;

					pContext->moveTo(CPoint(_xpos + (i * 2), (_ypos + height) - ((height - 14) * average)));
					pContext->lineTo(CPoint(_xpos + (i * 2), (_ypos + height) + ((height - 14) * average)));
				}
			}

			pContext->setFrameColor(CColor(10, 17, 9, 255));
			pContext->moveTo(CPoint(_xpos + ((wave->startSample / (float)wave->size) * (width - 2)) + 1, _ypos));
			pContext->lineTo(CPoint(_xpos + ((wave->startSample / (float)wave->size) * (width - 2)) + 1, _ypos + 34));

			pContext->setFrameColor(CColor(10, 17, 9, 255));
			pContext->moveTo(CPoint(_xpos + ((wave->endSample / (float)wave->size) * (width - 2)) + 3, _ypos));
			pContext->lineTo(CPoint(_xpos + ((wave->endSample / (float)wave->size) * (width - 2)) + 3, _ypos + 34));

		}

		UIWaveForm::setDirty(false);
	}
}

void UIWaveForm::reconnect()
{
}