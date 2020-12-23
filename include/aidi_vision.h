#ifndef AIDI_VISION_H
#define AIDI_VISION_H

#include "core/aidi_export.h"

#if !defined(__cplusplus) || defined(AIDI_USE_CABI)

#include "cabi/tools/c_aidi_entry.h"
#include "cabi/tools/c_aidi_templ.h"
#include "cabi/tools/c_task_editor.h"
#include "cabi/tools/c_evaluator.h"
#include "cabi/tools/c_aidi_filter.h"
#include "cabi/c_aidi_client.h"
#ifndef TX2
#include "cabi/c_aidi_trainer.h"
#endif
#include "cabi/c_aidi_image.h"
#include "cabi/c_aidi_label.h"

#elif !defined(WIN64) || (defined(_MSC_VER) && (_MSC_VER == 1800) && !defined(AIDI_USE_WRAPPED_CPPABI))

#include "core/tools/aidi_entry.h"
#include "core/tools/aidi_templ.h"
#include "core/tools/task_editor.h"
#include "core/tools/factory_editor.h"
#include "core/tools/aidi_filter.h"
#include "core/tools/evaluator.h"
#include "core/aidi_client.h"
#ifndef TX2
#include "core/aidi_trainer.h"
#endif
#include "core/aidi_image.h"
#include "core/aidi_label.h"

#else

#include "cppabi/tools/aidi_entry.h"
#include "cppabi/tools/aidi_templ.h"
#include "cppabi/tools/task_editor.h"
#include "cppabi/tools/evaluator.h"
#include "cppabi/tools/aidi_filter.h"
#include "cppabi/aidi_client.h"
#ifndef TX2
#include "cppabi/aidi_trainer.h"
#endif
#include "cppabi/aidi_image.h"
#include "cppabi/aidi_label.h"

#endif // abi

#endif // AIDI_VISION_H
