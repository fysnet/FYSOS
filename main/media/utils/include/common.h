/*
 *  common.h  v1.00       (C) Forever Young Software 1984-2015
 *
 */

// 'Allocates' some physical memory
bool get_physical_mapping(__dpmi_meminfo *mi, int *selector) {
  int sel;
  
  if (__dpmi_physical_address_mapping(mi) == -1)
    return FALSE;
  sel = __dpmi_allocate_ldt_descriptors(1);
  if (sel == -1)
    return FALSE;
  if (__dpmi_set_segment_base_address(sel, mi->address) == -1)
    return FALSE;
  if (__dpmi_set_segment_limit(sel, mi->size - 1))
    return FALSE;
  
  *selector = sel;
  return TRUE;
}

/*
///////////////////////////////////////////////////////////////////////////////////////////////
// This is the interrupt handler code.
//

_go32_dpmi_seginfo old_handler, new_handler;

#define IRQ_MASK_BIT(irq) ((irq) < 8 ? 1 << (irq) : (1 << (irq)) >> 8) 
#define IRQ_VECTOR(irq) ((irq) < 8 ? (irq) + 0x08 : (irq) - 8 + 0x70) 

void mask_pic(int irq) {
  int pic = (irq < 8) ? 0x21 : 0xA1;
  int irq_bit = IRQ_MASK_BIT(irq);
  outportb(pic, (inportb(pic) | irq_bit));
}

void unmask_pic(int irq) {
  int pic = (irq < 8) ? 0x21 : 0xA1;
  int irq_bit = ~IRQ_MASK_BIT(irq);
  outportb(pic, (inportb(pic) & irq_bit));
}

int set_interrupt_handler(const int irq, const int handler) {
  
  int ret = _go32_dpmi_get_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &old_handler);
  
  new_handler.pm_offset = handler;
  new_handler.pm_selector = _go32_my_cs();
  ret |= _go32_dpmi_allocate_iret_wrapper(&new_handler);
  ret |= _go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &new_handler);
  
  if (ret == 0)
    unmask_pic(irq);
  
  return ret;
}

int stop_interrupt_handler(const int irq) {
  mask_pic(irq);
  
  int ret = _go32_dpmi_free_iret_wrapper(&new_handler);
  ret |=    _go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &old_handler);
  
  return ret;
}
*/
