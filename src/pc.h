/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_pc_h_
#define __dj_include_pc_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING
#ifndef __STRICT_ANSI__
#ifndef _POSIX_SOURCE

int		kbhit(void);
int		getkey(void);	/* ALT's have 0x100 set */
int		getxkey(void);	/* ALT's have 0x100 set, 0xe0 sets 0x200 */

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_pc_h_ */
