#include "button.h"

#include "DiabloUI/art_draw.h"
//#include "DiabloUI/errorart.h"
#include "DiabloUI/text_draw.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

Art SmlButton;

void LoadSmlButtonArt()
{
	//LoadArt(&SmlButton, btnData, SML_BUTTON_WIDTH, SML_BUTTON_HEIGHT * 2, 2);
	LoadArt("ui_art\\smbutton.pcx", &SmlButton, 2);
}

DEVILUTION_END_NAMESPACE
