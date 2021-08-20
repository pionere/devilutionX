#include "button.h"

#include "DiabloUI/art.h"

DEVILUTION_BEGIN_NAMESPACE

Art SmlButton;

void LoadSmlButtonArt()
{
	//LoadArt(&SmlButton, btnData, SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT * 2, 2);
	LoadArt("ui_art\\smbutton.pcx", &SmlButton, 2);
}

DEVILUTION_END_NAMESPACE
