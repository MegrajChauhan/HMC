#ifndef _HMC_UTF8_
#define _HMC_UTF8_

#include <hmc_base_types.h>
#include <hmc_results.h>

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_encode(hwidechar_t *dest, hcodepoint_t src, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_encoding_length(hwidechar_t wc, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_required_encoding_length(hcodepoint_t cp, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_is_valid_encoding(hwidechar_t wc, u8_t *res);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_decode(hwidechar_t src, hcodepoint_t *dest, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL_SPECIFY(2, 3, 5) 
hmcResult_t hmc_utf8_encode_str(HMCAllocator *a, byteptr_t *dest, hcodepoint_t *src, hsize_t n, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL_SPECIFY(2, 3, 5)
hmcResult_t hmc_utf8_decode_str(HMCAllocator *a, byteptr_t src,
                              hcodepoint_t **dest, hsize_t n, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_str_is_valid(byteptr_t str, byteptr_t *ed);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_strlen(byteptr_t str, hsize_t *len);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_next_char(byteptr_t curr, byteptr_t str_st, byteptr_t *nxt);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_prev_char(byteptr_t curr, byteptr_t str_st, byteptr_t *prev);

HMC_ATTR_EXTERNAL HMC_ATTR_NO_DISCARD HMC_ATTR_NO_NULL 
hmcResult_t hmc_utf8_strcmp(byteptr_t str1, byteptr_t str2, hsize_t wcn, i8_t *res);

#endif
