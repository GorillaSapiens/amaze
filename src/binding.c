#include <stdlib.h>

#include "binding.h"

const char *action2verb(int action) {
   switch (action) {
      case ACTION_APPLY : return VERB_APPLY;
      case ACTION_QUAFF : return VERB_QUAFF;
      case ACTION_READ  : return VERB_READ;
      case ACTION_WEAR  : return VERB_WEAR;
      case ACTION_UNWEAR: return VERB_UNWEAR;
      default: return "?verb?";
   }
}
