#pragma once
#include "script_export_space.h"
#include "../xrRender/CameraBase.h"

typedef class_exporter<CCameraBase> CCameraScript;
add_to_type_list(CCameraScript)
#undef script_type_list
#define script_type_list save_type_list(CCameraScript)
