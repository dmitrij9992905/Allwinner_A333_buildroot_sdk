#ifndef A333_MEDIA_TRANSITION_H
#define A333_MEDIA_TRANSITION_H

/* Install after ui_init(); call tick on the same thread as EEZ/LVGL. */
void media_transition_init(void);
void media_transition_tick(void);

#endif
