/* SD -- square dance caller's helper.

    Copyright (C) 1990-1999  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 32. */


/* These are the bits that get filled in by "update_id_bits". */
#define BITS_TO_CLEAR (ID2_LEAD|ID2_TRAILER|ID2_BEAU|ID2_BELLE| \
ID2_FACING|ID2_NOTFACING|ID2_CENTER|ID2_END|ID2_CTR2|ID2_CTR6|ID2_OUTR2|ID2_OUTR6| \
ID2_CTRDMD|ID2_NCTRDMD|ID2_CTR1X4|ID2_NCTR1X4|ID2_CTR1X6|ID2_NCTR1X6| \
ID2_OUTR1X3|ID2_NOUTR1X3|ID2_CTR4|ID2_OUTRPAIRS)


/* These are the really global position bits.  They get filled in only at the top level. */
#define GLOB_BITS_TO_CLEAR (ID2_NEARCOL|ID2_NEARLINE|ID2_NEARBOX|ID2_FARCOL|ID2_FARLINE|ID2_FARBOX|ID2_FACEFRONT|ID2_FACEBACK|ID2_HEADLINE|ID2_SIDELINE)





/* Flags that reside in the "result_flags" word of a setup AFTER a call is executed.

   The low two bits tell, for a 2x2 setup after
   having a call executed, how that 2x2 is elongated in the east-west or
   north-south direction.  Since 2x2's are always totally canonical, the
   interpretation of the elongation direction is always absolute.  A 1 in this
   field means the elongation is east-west.  A 2 means the elongation is
   north-south.  A zero means the elongation is unknown.  These bits will be set
   whenever we know which elongation is preferred.  This will be the case if the
   setup was an elongated 2x2 prior to the call (because they are the ends), or
   the setup was a 1x4 prior to the call.  So, for example, after any recycle
   from a wave, these bits will be set to indicate that the dancers want to
   elongate the 2x2 perpendicular to their original wave.  These bits will be
   looked at only if the call was directed to the ends (starting from a grand
   wave, for example.)  Furthermore, the bits will be overridden if the
   "concentric" or "checkpoint" concepts were used, or if the ends did this
   as part of a concentrically defined call for which some forcing modifier
   such as "force_columns" was used.  Note that these two bits are in the same
   format as the "cmd.prior_elongation" field of a setup before the call is
   executed.  In many case, that field can be simply copied into the "result_flags"
   field.

   RESULTFLAG__DID_LAST_PART means that, when a sequentially defined call was executed
   with the "cmd_frac_flags" word nonzero, so that just one part was done,
   that part was the last part.  Hence, if we are doing a call with some "piecewise"
   or "random" concept, we do the parts of the call one at a time, with appropriate
   concepts on each part, until it comes back with this flag set.  This is used with
   RESULTFLAG__PARTS_ARE_KNOWN.

   RESULTFLAG__PARTS_ARE_KNOWN means that the bit in RESULTFLAG__DID_LAST_PART is
   valid.  Sometimes we are unable to obtain information about parts, because
   no one is doing the call.  This can legally happen, for example, on "interlocked
   parallelogram leads shakedown while the trailers star thru" from waves.  In each
   virtual 2x2 setup one or the other of the calls is being done by no one.

   RESULTFLAG__EXPAND_TO_2X3 means that a call was executed that takes a four person
   starting setup (typically a line) and yields a 2x3 result setup.  Two of those
   result spots will be phantoms, of course.  When recombining a divided setup in
   which such a call was executed, if the recombination wuold yield a 2x6, and if
   the center 4 spots would be empty, this flag directs divided_setup_move to
   collapse the setup to a 2x4.  This is what makes calls like "pair the line"
   and "step and slide" behave correctly from parallel lines and from a grand line.

   RESULTFLAG__NEED_DIAMOND means that a call has been executed with the "magic" or
   "interlocked" modifier, and it really should be changed to the "magic diamond"
   concept.  Otherwise, we might end up saying "magic diamond single wheel" when
   we should have said "magic diamond, diamond single wheel".

   RESULTFLAG__SPLIT_AXIS_MASK has info saying whether the call was split
   vertically (2) or horizontally (1) in "absolute space".  We can't say for
   sure whether the orientation is absolute, since a client may have stripped
   out rotation info.  What we mean is that it is relative to the incoming setup
   including the effect of its rotation field.  So, if the incoming setup was
   a 2x4 with rotation=0, that is, horizontally oriented in "absolute space",
   1 means it was done in 2x2's (split horizontally) and 2 means it was done
   in 1x4's (split vertically).  If the incoming setup was a 2x4 with
   rotation=1, that is, vertically oriented in "absolute space", 1 means it
   was done in 1x4's (split horizontally) and 2 means it was done in 2x2's
   (split vertically).  3 means that the call was a 1 or 2 person call, that
   could be split either way, so we have no information.

   RESULTFLAG__ACTIVE_PHANTOMS_ON and _OFF tell whether the state of the "active
   phantoms" mode flag was used in this sequence.  If it was read and found to
   be on, RESULTFLAG__ACTIVE_PHANTOMS_ON is set.  If it was read and found to
   be off, RESULTFLAG__ACTIVE_PHANTOMS_OFF is set.  It is, of course, read any
   time an "assume <whatever> concept is invoked.  This information is used to
   tell what kind of annotation to place in the transcript file to tell what
   assumptions were made.  The reason for having both bits is that the state
   of the active phantoms mode can be toggled at arbitrary times, so a single
   card might have both usages.  In that unusual case, we want to so indicate
   on the card.
*/

// The two low bits are used for result elongation, so we start with 0x00000004.
static const uint32 RESULTFLAG__DID_LAST_PART        = 0x00000004UL;
static const uint32 RESULTFLAG__PARTS_ARE_KNOWN      = 0x00000008UL;
static const uint32 RESULTFLAG__EXPAND_TO_2X3        = 0x00000010UL;
static const uint32 RESULTFLAG__NEED_DIAMOND         = 0x00000020UL;
static const uint32 RESULTFLAG__IMPRECISE_ROT        = 0x00000040UL;
// This is a six bit field.
static const uint32 RESULTFLAG__SPLIT_AXIS_FIELDMASK = 0x00001F80UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_XMASK     = 0x00000380UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_XBIT      = 0x00000080UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_YMASK     = 0x00001C00UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_YBIT      = 0x00000400UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_SEPARATION= 3;

static const uint32 RESULTFLAG__ACTIVE_PHANTOMS_ON   = 0x00002000UL;
static const uint32 RESULTFLAG__ACTIVE_PHANTOMS_OFF  = 0x00004000UL;
static const uint32 RESULTFLAG__SECONDARY_DONE       = 0x00008000UL;
static const uint32 RESULTFLAG__YOYO_FINISHED        = 0x00010000UL;
static const uint32 RESULTFLAG__TWISTED_FINISHED     = 0x00020000UL;
static const uint32 RESULTFLAG__SPLIT_FINISHED       = 0x00040000UL;
static const uint32 RESULTFLAG__NO_REEVALUATE        = 0x00080000UL;
static const uint32 RESULTFLAG__DID_Z_COMPRESSION    = 0x00100000UL;
static const uint32 RESULTFLAG__VERY_ENDS_ODD        = 0x00200000UL;
static const uint32 RESULTFLAG__VERY_CTRS_ODD        = 0x00400000UL;

typedef struct glonk {
   char txt[MAX_TEXT_LINE_LENGTH];
   struct glonk *nxt;
} comment_block;

/* These bits appear in the "concparseflags" word. */
/* This is a duplicate, and exists only to make menus nicer.
   Ignore it when scanning in parser. */
#define CONCPARSE_MENU_DUP       0x00000001UL
/* If the parse turns out to be ambiguous, don't use this one --
   yield to the other one. */
#define CONCPARSE_YIELD_IF_AMB   0x00000002UL
/* Parse directly.  It directs the parser to allow this concept
   (and similar concepts) and the following call to be typed
   on one line.  One needs to be very careful about avoiding
   ambiguity when setting this flag. */
#define CONCPARSE_PARSE_DIRECT   0x00000004UL
/* These are used by "print_recurse" in sdutil.c to control the printing.
   They govern the placement of commas. */
#define CONCPARSE_PARSE_L_TYPE 0x8
#define CONCPARSE_PARSE_F_TYPE 0x10
#define CONCPARSE_PARSE_G_TYPE 0x20


/* BEWARE!!  This list must track the array "warning_strings" in sdtables.c . */
typedef enum {
   warn__none,
   warn__do_your_part,
   warn__tbonephantom,
   warn__awkward_centers,
   warn__bad_concept_level,
   warn__not_funny,
   warn__hard_funny,
   warn__rear_back,
   warn__awful_rear_back,
   warn__excess_split,
   warn__lineconc_perp,
   warn__dmdconc_perp,
   warn__lineconc_par,
   warn__dmdconc_par,
   warn__xclineconc_perpc,
   warn__xcdmdconc_perpc,
   warn__xclineconc_perpe,
   warn__ctrstand_endscpls,
   warn__ctrscpls_endstand,
   warn__each2x2,
   warn__each1x4,
   warn__each1x2,
   warn__take_right_hands,
   warn__ctrs_are_dmd,
   warn__1_4_pgram,
   warn__full_pgram,
   warn__3_4_pgram,
   warn__offset_gone,
   warn__overlap_gone,
   warn__to_o_spots,
   warn__to_x_spots,
   warn__check_butterfly,
   warn__check_galaxy,
   warn__some_rear_back,
   warn__not_tbone_person,
   warn__check_c1_phan,
   warn__check_dmd_qtag,
   warn__check_quad_dmds,
   warn__check_3x4,
   warn__check_2x4,
   warn__check_4x4,
   warn__check_hokey_4x4,
   warn__check_4x4_start,
   warn__check_pgram,
   warn__ctrs_stay_in_ctr,
   warn__check_c1_stars,
   warn__check_gen_c1_stars,
   warn__bigblock_feet,
   warn__bigblockqtag_feet,
   warn__diagqtag_feet,
   warn__adjust_to_feet,
   warn__some_touch,
   warn__split_to_2x4s,
   warn__split_to_2x3s,
   warn__split_to_1x8s,
   warn__split_to_1x6s,
   warn__take_left_hands,
   warn__evil_interlocked,
   warn__split_phan_in_pgram,
   warn__bad_interlace_match,
   warn__not_on_block_spots,
   warn__stupid_phantom_clw,
   warn__bad_modifier_level,
   warn__bad_call_level,
   warn__did_not_interact,
   warn__opt_for_normal_cast,
   warn__opt_for_normal_hinge,
   warn__opt_for_2fl,
   warn_partial_solomon,
   warn_same_z_shear,
   warn__like_linear_action,
   warn__no_z_action,
   warn__phantoms_thinner,
   warn__hokey_jay_shapechanger,
   warn__split_1x6,
   warn_interlocked_to_6,
   warn__colocated_once_rem,
   warn_big_outer_triangles,
   warn_hairy_fraction,
   warn_bad_collision,
   warn__dyp_resolve_ok,
   warn__unusual,
   warn_controversial,
   warn_serious_violation,
   warn_bogus_yoyo_rims_hubs,
   warn_pg_in_2x6,
   warn__tasteless_com_spot,
   warn__tasteless_junk,
   warn__tasteless_slide_thru  /* If this ceases to be last, look 2 lines below! */
} warning_index;
#define NUM_WARNINGS (((int) warn__tasteless_slide_thru)+1)

/* BEWARE!!  This list must track the array "resolve_table" in sdgetout.c . */
typedef enum {
   resolve_none,
   resolve_rlg,
   resolve_la,
   resolve_ext_rlg,
   resolve_ext_la,
   resolve_slipclutch_rlg,
   resolve_slipclutch_la,
   resolve_circ_rlg,
   resolve_circ_la,
   resolve_pth_rlg,
   resolve_pth_la,
   resolve_tby_rlg,
   resolve_tby_la,
   resolve_xby_rlg,
   resolve_xby_la,
   resolve_dixie_grand,
   resolve_prom,
   resolve_revprom,
   resolve_sglfileprom,
   resolve_revsglfileprom,
   resolve_circle
} resolve_kind;

typedef struct {
   resolve_kind kind;
   int distance;
} resolve_indicator;


/* These bits are used to allocate flag bits
   that appear in the "callflagsf" word of a top level callspec_block
   and the "cmd_final_flags.final" of a setup with its command block. */

/* A 3-bit field. */
#define CFLAGH__TAG_CALL_RQ_MASK          0x00000007UL
#define CFLAGH__TAG_CALL_RQ_BIT           0x00000001UL
#define CFLAGH__REQUIRES_SELECTOR         0x00000008UL
#define CFLAGH__REQUIRES_DIRECTION        0x00000010UL
#define CFLAGH__CIRC_CALL_RQ_BIT          0x00000020UL
#define CFLAGH__ODD_NUMBER_ONLY           0x00000040UL
#define CFLAGHSPARE_1                     0x00000080UL
#define CFLAGHSPARE_2                     0x00000100UL
#define CFLAGHSPARE_3                     0x00000200UL
#define CFLAGHSPARE_4                     0x00000400UL
#define CFLAGHSPARE_5                     0x00000800UL
#define CFLAGHSPARE_6                     0x00001000UL
/* We need to leave the top 8 bits free in order to accomodate the "CFLAG2" bits. */

/* These flags, and "CFLAGH__???" flags, go along for the ride, in the callflagsf
   word of a callspec.  We use symbols that have been graciously
   provided for us from database.h to tell us what bits may be safely used next
   to the heritable flags.  Note that these bits overlap the FINAL__?? bits above.
   These are used in the callflagsf word of a callspec.  The FINAL__?? bits are
   used elsewhere.  They don't mix. */

#define ESCAPE_WORD__LEFT                 CFLAGHSPARE_1
#define ESCAPE_WORD__CROSS                CFLAGHSPARE_2
#define ESCAPE_WORD__MAGIC                CFLAGHSPARE_3
#define ESCAPE_WORD__INTLK                CFLAGHSPARE_4

/* These flags go into the "concept_prop" field of a "concept_table_item".

   CONCPROP__SECOND_CALL means that the concept takes a second call, so a sublist must
      be created, with a pointer to same just after the concept pointer.

   CONCPROP__USE_SELECTOR means that the concept requires a selector, which must be
      inserted into the concept list just after the concept pointer.

   CONCPROP__NEEDK_4X4   mean that the concept requires the indicated setup, and, at
   CONCPROP__NEEDK_2X8   the top level, the existing setup should be expanded as needed.
      etc....

   CONCPROP__SET_PHANTOMS means that phantoms are in use under this concept, so that,
      when looking for tandems or couples, we shouldn't be disturbed if we
      pair someone with a phantom.  It is what makes "split phantom lines tandem"
      work, so that "split phantom lines phantom tandem" is unnecessary.

   CONCPROP__NO_STEP means that stepping to a wave or rearing back from one is not
      allowed under this concept.

   CONCPROP__GET_MASK means that tbonetest & livemask need to be computed before executing the concept.

   CONCPROP__STANDARD means that the concept can be "standard".

   CONCPROP__USE_NUMBER         If a concept takes one number, only CONCPROP__USE_NUMBER is set.
   CONCPROP__USE_TWO_NUMBERS    If it takes two numbers both bits are set.

   CONCPROP__SHOW_SPLIT means that the concept prepares the "split_axis" bits properly
      for transmission back to the client.  Normally this is off, and the split axis bits
      will be cleared after execution of the concept.

   CONCPROP__NEED_ARG2_MATRIX means that the "arg2" word of the concept_descriptor
      block contains additional bits of the "CONCPROP__NEEDK_3X8" kind to be sent to
      "do_matrix_expansion".  This is done so that concepts with different matrix
      expansion bits can share the same concept type.
*/



#define CONCPROP__SECOND_CALL      0x00000001UL
#define CONCPROP__USE_SELECTOR     0x00000002UL
#define CONCPROP__SET_PHANTOMS     0x00000004UL
#define CONCPROP__NO_STEP          0x00000008UL

/* This is a five bit field.  CONCPROP__NEED_LOBIT marks its low bit.
   WARNING!!!  The values in this field are encoded into a bit field
   for the setup expansion/normalization tables.  It follows that
   there can't be more than 32 of them. */
#define CONCPROP__NEED_LOBIT       0x00000010UL
/* A bit mask for the "NEEDK" bits. */
#define CONCPROP__NEED_MASK        0x000001F0UL

#define CONCPROP__NEEDK_4X4        0x00000010UL
#define CONCPROP__NEEDK_2X8        0x00000020UL
#define CONCPROP__NEEDK_2X6        0x00000030UL
#define CONCPROP__NEEDK_4DMD       0x00000040UL
#define CONCPROP__NEEDK_BLOB       0x00000050UL
#define CONCPROP__NEEDK_4X6        0x00000060UL
#define CONCPROP__NEEDK_3X8        0x00000070UL
#define CONCPROP__NEEDK_3DMD       0x00000080UL
#define CONCPROP__NEEDK_1X10       0x00000090UL
#define CONCPROP__NEEDK_1X12       0x000000A0UL
#define CONCPROP__NEEDK_3X4        0x000000B0UL
#define CONCPROP__NEEDK_1X16       0x000000C0UL
#define CONCPROP__NEEDK_QUAD_1X4   0x000000D0UL
#define CONCPROP__NEEDK_TWINDMD    0x000000E0UL
#define CONCPROP__NEEDK_TWINQTAG   0x000000F0UL
#define CONCPROP__NEEDK_CTR_DMD    0x00000100UL
#define CONCPROP__NEEDK_END_DMD    0x00000110UL
#define CONCPROP__NEEDK_TRIPLE_1X4 0x00000120UL
#define CONCPROP__NEEDK_CTR_1X4    0x00000130UL
#define CONCPROP__NEEDK_END_1X4    0x00000140UL
#define CONCPROP__NEEDK_CTR_2X2    0x00000150UL
#define CONCPROP__NEEDK_END_2X2    0x00000160UL
#define CONCPROP__NEEDK_3X4_D3X4   0x00000170UL
#define CONCPROP__NEEDK_3X6        0x00000180UL
#define CONCPROP__NEEDK_4D_4PTPD   0x00000190UL
#define CONCPROP__NEEDK_4X5        0x000001A0UL
#define CONCPROP__NEEDK_2X12       0x000001B0UL

#define CONCPROP__NEED_ARG2_MATRIX 0x00000200UL                                   
/* spare:                          0x00000400UL */
/* spare:                          0x00000800UL */
/* spare:                          0x00010000UL */
/* spare:                          0x00020000UL */
/* spare:                          0x00040000UL */
/* spare:                          0x00080000UL */
/* spare:                          0x00100000UL */
#define CONCPROP__GET_MASK         0x00200000UL
#define CONCPROP__STANDARD         0x00400000UL
#define CONCPROP__USE_NUMBER       0x00800000UL
#define CONCPROP__USE_TWO_NUMBERS  0x01000000UL
#define CONCPROP__MATRIX_OBLIVIOUS 0x02000000UL
#define CONCPROP__PERMIT_MATRIX    0x04000000UL
#define CONCPROP__SHOW_SPLIT       0x08000000UL
#define CONCPROP__PERMIT_MYSTIC    0x10000000UL
#define CONCPROP__PERMIT_REVERSE   0x20000000UL

/* This allows 96 warnings. */
/* BEWARE!!  If this is changed, this initializers for things like "no_search_warnings"
   in sdmain.c will need to be updated. */
#define WARNING_WORDS 3

typedef struct {
   uint32 bits[WARNING_WORDS];
} warning_info;

typedef struct {           /* This record is one state in the evolving sequence. */
   parse_block *command_root;
   setup state;
   resolve_indicator resolve_flag;
   long_boolean draw_pic;
   warning_info warnings;
   int centersp;           /* only nonzero for history[1] */
   int text_line;          /* how many lines of text existed after this item was written,
                              only meaningful if "written_history_items" is >= this index */
} configuration;

typedef struct {
   uint32 concept_prop;      /* Takes bits of the form CONCPROP__??? */
   void (*concept_action)(setup *, parse_block *, setup *);
} concept_table_item;

typedef enum {
   error_flag_none = 0,          /* Must be zero because setjmp returns this. */
   error_flag_1_line,            /* 1-line error message, text is in error_message1. */
   error_flag_2_line,            /* 2-line error message, text is in error_message1 and
                                    error_message2. */
   error_flag_collision,         /* collision error, message is that people collided, they are in
                                    collision_person1 and collision_person2. */
   error_flag_wrong_resolve_command, /* "resolve" or similar command was called
                                         in inappropriate context, text is in error_message1. */
   error_flag_wrong_command,     /* clicked on something inappropriate in subcall reader. */
   error_flag_cant_execute,      /* unable-to-execute error, person is in collision_person1,
                                    text is in error_message1. */
   error_flag_show_stats,        /* wants to display stale call statistics. */
   error_flag_selector_changed,  /* warn that selector was changed during clipboard paste. */
   error_flag_formation_changed  /* warn that formation changed during clipboard paste. */
} error_flag_type;

/* These enumerate the setups from which we can perform a "normalize" search. */
/* This list tracks the array "nice_setup_info". */

typedef enum {
   nice_start_4x4,
   nice_start_3x4,
   nice_start_2x8,
   nice_start_2x6,
   nice_start_1x10,
   nice_start_1x12,
   nice_start_1x14,
   nice_start_1x16,
   nice_start_3dmd,
   nice_start_4dmd,
   nice_start_4x6
} nice_start_kind;
#define NUM_NICE_START_KINDS (((int) nice_start_4x6)+1)

typedef struct {
   int *full_list;
   int *on_level_list;
   int full_list_size;
} nice_setup_thing;

typedef struct {
   setup_kind kind;
   nice_setup_thing *thing;
   int *array_to_use_now;
   int number_available_now;
} nice_setup_info_item;

typedef enum {
   resolve_goodness_only_nice,
   resolve_goodness_always,
   resolve_goodness_maybe
} resolve_goodness_test;

#define zig_zag_level l_a2
#define cross_by_level l_c1
#define dixie_grand_level l_plus
#define extend_34_level l_plus
#define phantom_tandem_level l_c4a
#define intlk_triangle_level l_c2
#define beau_belle_level l_a2


typedef struct {
   char *destcurr;
   char lastchar;
   char lastlastchar;
   char *lastblank;
   long_boolean usurping_writechar;
} writechar_block_type;

typedef struct {
   void * (*get_mem_fn)(uint32 siz);
   void (*uims_database_error_fn)(Cstring message, Cstring call_name);
   parse_block * (*get_parse_block_fn)(void);
   void (*writechar_fn)(char src);
   void (*writestuff_fn)(Const char *s);
   NORETURN1 void (*do_throw_fn)(error_flag_type f) NORETURN2;
   long_boolean (*do_subcall_query_fn)(
      int snumber,
      parse_block *parseptr,
      parse_block **newsearch,
      long_boolean this_is_tagger,
      long_boolean this_is_tagger_circcer,
      callspec_block *orig_call);
   parse_state_type *parse_state_ptr;
   writechar_block_type *writechar_block_ptr;
} callbackstuff;

/* VARIABLES */

extern SDLIB_API callbackstuff the_callback_block;                  /* in SDTOP */
extern SDLIB_API char error_message1[MAX_ERR_LENGTH];               /* in SDTOP */
extern SDLIB_API char error_message2[MAX_ERR_LENGTH];               /* in SDTOP */
extern SDLIB_API uint32 collision_person1;                          /* in SDTOP */
extern SDLIB_API uint32 collision_person2;                          /* in SDTOP */
extern SDLIB_API configuration *history;                            /* in SDTOP */
extern SDLIB_API int history_ptr;                                   /* in SDTOP */
extern SDLIB_API int written_history_items;                         /* in SDTOP */
extern SDLIB_API int written_history_nopic;                         /* in SDTOP */
extern SDLIB_API concept_table_item concept_table[];                /* in SDCONCPT */

extern SDLIB_API nice_setup_info_item nice_setup_info[];            /* in SDCTABLE */
extern SDLIB_API int phantom_concept_index;                         /* in SDCTABLE */
extern SDLIB_API int matrix_2x8_concept_index;                      /* in SDCTABLE */
extern SDLIB_API int cross_concept_index;                           /* in SDCTABLE */
extern SDLIB_API int magic_concept_index;                           /* in SDCTABLE */
extern SDLIB_API int intlk_concept_index;                           /* in SDCTABLE */
extern SDLIB_API int left_concept_index;                            /* in SDCTABLE */
extern int grand_concept_index;                                     /* in SDCTABLE */
extern SDLIB_API int general_concept_offset;                        /* in SDCTABLE */
extern SDLIB_API int general_concept_size;                          /* in SDCTABLE */

extern SDLIB_API Cstring warning_strings[];                         /* in SDTABLES */
extern SDLIB_API setup_attr setup_attrs[];                          /* in SDTABLES */
extern SDLIB_API int begin_sizes[];                                 /* in SDTABLES */
extern SDLIB_API startinfo startinfolist[];                         /* in SDTABLES */

extern SDLIB_API int concept_sublist_sizes[NUM_CALL_LIST_KINDS];
extern SDLIB_API short int *concept_sublists[NUM_CALL_LIST_KINDS];
extern SDLIB_API int good_concept_sublist_sizes[NUM_CALL_LIST_KINDS];
extern SDLIB_API short int *good_concept_sublists[NUM_CALL_LIST_KINDS];

extern SDLIB_API long_boolean selector_used;                        /* in SDPREDS */
extern SDLIB_API long_boolean number_used;                          /* in SDPREDS */
extern SDLIB_API long_boolean mandatory_call_used;                  /* in SDPREDS */
extern SDLIB_API predicate_descriptor pred_table[];                 /* in SDPREDS */
extern SDLIB_API int selector_preds;                                /* in SDPREDS */

/* In SDMOVES */

SDLIB_API void canonicalize_rotation(setup *result);

/* In SDTOP */

SDLIB_API void initialize_sdlib(void);
SDLIB_API long_boolean check_for_concept_group(
   Const parse_block *parseptrcopy,
   long_boolean want_all_that_other_stuff,
   concept_kind *k_p,
   uint32 *need_to_restrain_p,   /* 1=(if not doing echo), 2=(yes, always) */
   parse_block **parseptr_skip_p);
NORETURN1 SDLIB_API void fail(Const char s[]) NORETURN2;
NORETURN1 SDLIB_API void specialfail(Const char s[]) NORETURN2;
SDLIB_API callarray *assoc(begin_kind key, setup *ss, callarray *spec);
SDLIB_API uint32 rotperson(uint32 n, int amount);
SDLIB_API parse_block *process_final_concepts(
   parse_block *cptr,
   long_boolean check_errors,
   uint64 *final_concepts);
SDLIB_API resolve_indicator resolve_p(setup *s);
SDLIB_API long_boolean warnings_are_unacceptable(long_boolean strict);
SDLIB_API void toplevelmove(void);
SDLIB_API void finish_toplevelmove(void);
SDLIB_API Const char *get_escape_string(char c);
SDLIB_API void unparse_call_name(Cstring name, char *s, call_conc_option_state *options);
SDLIB_API void print_recurse(parse_block *thing, int print_recurse_arg);
