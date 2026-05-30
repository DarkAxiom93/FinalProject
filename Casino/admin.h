#ifndef ADMIN_H
#define ADMIN_H

#include "casino.h"

// ==========================================
// מתג גרסת מפתחים (Developer Mode Switch)
// לגרסת פרודקשן למשתמשים, הוסף // לפני השורה הבאה:
#define ENABLE_ADMIN_PANEL 

#ifdef ENABLE_ADMIN_PANEL
#define ADMIN_ENTRY_CODE 8426 
#endif
// ==========================================

void admin_panel(Player* p);

#endif