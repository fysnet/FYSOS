/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * slider.cpp
 *  
 *  used by the scroll object
 */

#include "gui.h"

/*  slider_set()
 *       slider = pointer to the slider object
 *        range = give the range of the slider
 *         size = size of the slider
 *        value = position of the slider
 *
 *  sets a slider to a specified size, range, and value
 *   
 */
void slider_set(struct SLIDER *slider, int range, int size, int value) {
  int highest;
  
  if (range != GUIDEF)
    slider->range = range;
  if (size != GUIDEF)
    slider->size = size;
  if (value != GUIDEF)
    slider->value = value;
  
  highest = slider->range - slider->size;
  
  if (slider->value > highest)
    slider->value = highest;
  if (slider->value < 0)
    slider->value = 0;
  
  slider->upper = 0;
  slider->lower = slider->dim + 1;
  
  if (slider->range) {
    const int range = (slider->dim + 1) - slider->min;
    const int thumb = slider->size * range / slider->range;
    const int index = slider->value * range / slider->range;
    
    slider->upper = index;
    slider->lower = slider->upper + thumb + slider->min;
  }
}

/*  slider_to()
 *       slider = pointer to the slider object
 *        value = position of the slider
 *
 *  moves a slider to a specified position
 *   
 */
void slider_to(struct SLIDER *slider, int value) {
  const int oldvalue = slider->value;
  
  slider_set(slider, GUIDEF, GUIDEF, value);
  
  if (slider->value != oldvalue) {
    if (slider->dim == gui_w(slider))
      emit(HSCROLL, NULL);
    else if (slider->dim == gui_h(slider))
      emit(VSCROLL, NULL);
    
    obj_dirty(GUIOBJ(slider), TRUE);
  }
}

/*  slider_value()
 *       slider = pointer to the slider object
 *
 *  returns the current slider position
 *   
 */
int slider_value(struct SLIDER *slider) {
  return slider->value;
}

/* vslider_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 */
void vslider_class(void) {
  struct SLIDER *slider = (struct SLIDER *) eventstack.object;
  const struct POINTER_INFO *info = pointer_info();
  
  switch (eventstack.event) {
    // draw the slider to the screen
    case EXPOSE:
      gui_vslider(slider);
      return;
    
    // allow the user to grab the slider and move it
    case ARM:
      obj_dirty(GUIOBJ(slider), TRUE);
      obj_class();
      obj_wantmove(GUIOBJ(slider), obj_armed(GUIOBJ(slider)));
      return;
      
    // wheel mouse movement moves slider also
    case MPOINTER_MOVE:
      slider_to(slider, slider->value + (info->dz * 5));
      break;
      
    // moving slider with mouse
    case LPOINTER_HELD:
      if (info->y <= (gui_top(slider) + SLIDER_SIZE)) {
        slider_to(slider, slider->value - 2);
        return;
      } else if (info->y >= (gui_bottom(slider) - SLIDER_SIZE)) {
        slider_to(slider, slider->value + 2);
        return;
      }
      break;
      
    // if one of the "buttons" are pressed, move it two indexs,
    //  else if we click in an area where the slider is absent,
    //  move the slider to that side.
    case LPOINTER_PRESS:
      if (info->y <= (gui_top(slider) + SLIDER_SIZE)) {
        slider_to(slider, slider->value - 2);
        return;
      } else if (info->y < (gui_top(slider) + slider->upper + SLIDER_SIZE)) {
        slider_to(slider, slider->value - slider->size);
        return;
      } else if (info->y >= (gui_bottom(slider) - SLIDER_SIZE)) {
        slider_to(slider, slider->value + 2);
        return;
      } else if (info->y > (gui_top(slider) + slider->lower - SLIDER_SIZE)) {
        slider_to(slider, slider->value + slider->size);
        return;
      }
      
      pointer_hold();
      break;
      
    // we released the slider, don't move it any more
    case LPOINTER_RELEASE:
      obj_dirty(GUIOBJ(slider), TRUE);
      pointer_release();
      break;
    
    // if we are dragging the slider, move it along
    case POINTER_MOVE:
      if (obj_armed(GUIOBJ(slider))) {
        const struct POINTER_INFO *info = pointer_info();
        if (info->dy && slider->range) {
           const int range = gui_h(slider) - 2 - slider->min;
           const int dy = info->dy * slider->range / range;
           slider_to(slider, slider->value + dy);
        }
      }
      break;
      
    // give the slider a size
    case GEOMETRY:
      slider->dim = gui_h(slider);
      slider_set(slider, GUIDEF, GUIDEF, GUIDEF);
      return;
      
    // give the slider a default size
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      rect->right = rect->left + SLIDER_SIZE;
    } return;
  }
  
  // else call the object class
  obj_class();
}

/*  obj_vslider()
 *       slider = pointer to the slider object
 *         size = size of memory to allocate
 *       parent = parent object (scroll object)
 *        theid = the id for the slider (ignored)
 *
 *  create a vert slider
 *   
 */
struct SLIDER *obj_vslider(struct SLIDER *slider, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct SLIDER);
  
  slider = (struct SLIDER *) object(GUIOBJ(slider), vslider_class, size, parent, theid);
  if (slider) {
    // armable and selectable
    obj_armable(GUIOBJ(slider), TRUE);
    obj_selectable(GUIOBJ(slider), TRUE);
    slider->min = 5;
  }
  
  return slider;
}

/* hslider_class()
 *   no parameters
 *
 *   called by GUI system when an event is used
 *  
 *   eventstack.event = current event
 *   eventstack.object = pointer to list
 *   eventstack.data = rect of object
 */
void hslider_class(void) {
  struct SLIDER *slider = (struct SLIDER *) eventstack.object;
  const struct POINTER_INFO *info = pointer_info();
  
  switch (eventstack.event) {
    // draw the slider to the screen
    case EXPOSE:
      gui_hslider(slider);
      return;

    // allow the user to grab the slider and move it
    case ARM:
      obj_dirty(GUIOBJ(slider), TRUE);
      obj_class();
      obj_wantmove(GUIOBJ(slider), obj_armed(GUIOBJ(slider)));
      return;
      
    // wheel mouse movement moves slider also
    case MPOINTER_MOVE:
      slider_to(slider, slider->value + (info->dz * 5));
      break;
      
    // moving slider with mouse
    case LPOINTER_HELD:
      if (info->x <= (gui_left(slider) + SLIDER_SIZE)) {
        slider_to(slider, slider->value - 2);
        return;
      } else if (info->x >= (gui_right(slider) - SLIDER_SIZE)) {
        slider_to(slider, slider->value + 2);
        return;
      }
      break;
      
    // if one of the "buttons" are pressed, move it two indexs,
    //  else if we click in an area where the slider is absent,
    //  move the slider to that side.
    case LPOINTER_PRESS:
      if (info->x <= (gui_left(slider) + SLIDER_SIZE)) {
        slider_to(slider, slider->value - 2);
        return;
      } else if (info->x < (gui_left(slider) + slider->upper + SLIDER_SIZE)) {
        slider_to(slider, slider->value - slider->size);
        return;
      } else if (info->x >= (gui_right(slider) - SLIDER_SIZE)) {
        slider_to(slider, slider->value + 2);
        return;
      } else if (info->x > (gui_left(slider) + slider->lower - SLIDER_SIZE)) {
        slider_to(slider, slider->value + slider->size);
        return;
      }
      
      pointer_hold();
      break;
      
    // we released the slider, don't move it any more
    case LPOINTER_RELEASE:
      obj_dirty(GUIOBJ(slider), TRUE);
      pointer_release();
      break;
      
    // if we are dragging the slider, move it along
    case POINTER_MOVE:
      if (obj_armed(GUIOBJ(slider))) {
        const struct POINTER_INFO *info = pointer_info();
        if (info->dx && slider->range) {
          const int range = gui_w(slider) - 2 - slider->min;
          const int dx = info->dx * slider->range / range;
          slider_to(slider, slider->value + dx);
        }
      }
      break;
      
    // give the slider a size
    case GEOMETRY:
      slider->dim = gui_w(slider);
      slider_set(slider, slider->range, slider->size, slider->value);
      return;

    // give the slider a default size
    case DEFAULTRECT: {
      struct RECT *rect = defaultrect_data();
      rect->bottom = rect->top + SLIDER_SIZE;
      return;
    }
  }
  
  // else call the object class
  obj_class();
}

/*  obj_hslider()
 *       slider = pointer to the slider object
 *         size = size of memory to allocate
 *       parent = parent object (scroll object)
 *        theid = the id for the slider (ignored)
 *
 *  create a horz slider
 *   
 */
struct SLIDER *obj_hslider(struct SLIDER *slider, bit32u size, struct OBJECT *parent, int theid) {
  MINSIZE(size, struct SLIDER);
  
  slider = (struct SLIDER *) object(GUIOBJ(slider), hslider_class, size, parent, theid);
  if (slider) {
    // armable and selectable
    obj_armable(GUIOBJ(slider), TRUE);
    obj_selectable(GUIOBJ(slider), TRUE);
    slider->min = 5;
  }
  
  return slider;
}
