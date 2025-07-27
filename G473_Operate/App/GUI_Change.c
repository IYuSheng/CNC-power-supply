#include "Gui_Change.h"

void Gui_Event_Data(void)
{
	static uint8_t t;
	t++;
	if(t > 254)
	{
		t = 0;
		counter++;
	}
		fr_printf("Count: %lu", counter);
    lv_label_set_text_fmt(label_counter, "Count: %lu", counter);
	
	lv_obj_invalidate(label_counter);  // 强制标记为脏区域，触发刷新
}
