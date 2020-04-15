#ifndef _CENGINE_UI_LAYOUT_ALIGNMENT_H_
#define _CENGINE_UI_LAYOUT_ALIGNMENT_H_

typedef enum AxisAlignment {

	ALIGN_NONE           		= 0,    // no alignment

	ALIGN_AUTO           		= 1,    // element size will be modified automatically
	ALIGN_KEEP_SIZE      		= 2,    // set element size will be kept

	ALIGN_PADDING_ALL    		= 3,    // padding between edges and elements
	ALIGN_PADDING_BETWEEN		= 4,    // only padding between elements

} AxisAlignment;

#endif