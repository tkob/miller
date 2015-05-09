// Functions involving more than one container type
#ifndef MIXUTIL_H
#define MIXUTIL_H
#include "containers/lrec.h"
#include "containers/slls.h"
slls_t* mlr_keys_from_record(lrec_t* prec);
slls_t* mlr_selected_values_from_record(lrec_t* prec, slls_t* pselected_field_names);
#endif // MIXUTIL_H