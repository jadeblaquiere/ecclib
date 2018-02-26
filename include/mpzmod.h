//BSD 3-Clause License
//
//Copyright (c) 2017, jadeblaquiere
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//
//* Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//* Neither the name of the copyright holder nor the names of its
//  contributors may be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _EC_MPZMOD_H_INCLUDED_
#define _EC_MPZMOD_H_INCLUDED_

#define _EC_MPZMOD_H_INLINE_MATH

#include <assert.h>
#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNLIKELY(cond)                 __GMP_UNLIKELY(cond)

#define SIZ(x) ((x)->_mp_size)
#define PTR(x) ((x)->_mp_d)
#define EXP(x) ((x)->_mp_exp)
#define ALLOC(x) ((x)->_mp_alloc)

#define MPZ_SRCPTR_SWAP(x, y)						\
  do {									\
    mpz_srcptr __mpz_srcptr_swap__tmp = (x);				\
    (x) = (y);								\
    (y) = __mpz_srcptr_swap__tmp;					\
  } while (0)

#define MP_SIZE_T_SWAP(x, y)						\
  do {									\
    mp_size_t __mp_size_t_swap__tmp = (x);				\
    (x) = (y);								\
    (y) = __mp_size_t_swap__tmp;					\
  } while (0)

#define MPZ_REALLOC(z,n) (UNLIKELY ((n) > ALLOC(z))			\
			  ? (mp_ptr) _mpz_realloc(z,n)			\
			  : PTR(z))

#define MPN_NORMALIZE(DST, NLIMBS) \
  do {									\
    while ((NLIMBS) > 0)						\
      {									\
	if ((DST)[(NLIMBS) - 1] != 0)					\
	  break;							\
	(NLIMBS)--;							\
      }									\
  } while (0)

#ifdef _EC_MPZMOD_H_INLINE_MATH

static inline void _mpn_modadd (mpz_ptr w, mpz_srcptr u, mpz_srcptr v, mpz_srcptr p)
{
  mp_srcptr up, vp, pp;
  mp_ptr wp;
  mp_size_t usize, vsize, psize, wsize;

  usize = SIZ(u);
  vsize = SIZ(v);
  psize = SIZ(p);

  assert(usize >= 0);
  assert(vsize >= 0);
  assert(psize >= 0);

  if (usize < vsize)
    {
      /* Swap U and V. */
      MPZ_SRCPTR_SWAP (u, v);
      MP_SIZE_T_SWAP (usize, vsize);
    }

  /* True: ABS_USIZE >= ABS_VSIZE.  */

  /* If not space for w (and possible carry), increase space.  */
  wsize = usize + 1;
  wp = MPZ_REALLOC (w, wsize);

  /* These must be after realloc (u or v may be the same as w).  */
  up = PTR(u);
  vp = PTR(v);
  pp = PTR(p);

    {
      /* U and V have same sign.  Add them.  */
      mp_limb_t cy_limb = mpn_add(wp, up, usize, vp, vsize);
      wp[usize] = cy_limb;
      wsize = usize + cy_limb;
      SIZ(w) = wsize;
      if ((wsize > psize) || ((wsize == psize) && (mpn_cmp(wp, pp, wsize) >= 0))) {
          mpn_sub(wp, wp, wsize, pp, psize);
	      MPN_NORMALIZE (wp, wsize);
          SIZ(w) = wsize;
      }
    }
}

static inline void _mpn_modsub (mpz_ptr w, mpz_srcptr u, mpz_srcptr v, mpz_srcptr p)
{
  mp_srcptr up, vp, pp;
  mp_ptr wp;
  mp_size_t usize, vsize, psize, wsize;

  usize = SIZ(u);
  vsize = SIZ(v);
  psize = SIZ(p);

  assert(usize >= 0);
  assert(vsize >= 0);
  assert(psize >= 0);

  /* If not space for w (and possible carry), increase space.  */
  wsize = psize + 1;
  wp = MPZ_REALLOC (w, wsize);

  /* These must be after realloc (u or v may be the same as w).  */
  up = PTR(u);
  vp = PTR(v);
  pp = PTR(p);

  if ((usize > vsize) || ((usize == vsize) && (mpn_cmp(up, vp, usize) >= 0))) {
      // simple subtraction
      mp_limb_t bw_limb = mpn_sub(wp, up, usize, vp, vsize);
      assert(bw_limb == 0);
      wsize = usize;
      MPN_NORMALIZE (wp, wsize);
      SIZ(w) = wsize;
  } else {
      // underflow -> (u-v)+p = p+(u-v) = p-(v-u)
      mp_limb_t bw_limb = mpn_sub(wp, vp, vsize, up, usize);
      assert(bw_limb == 0);
      wsize = vsize;
      MPN_NORMALIZE (wp, wsize);
      SIZ(w) = wsize;
      bw_limb = mpn_sub(wp, pp, psize, wp, wsize);
      assert(bw_limb == 0);
      wsize = psize;
      MPN_NORMALIZE (wp, wsize);
      SIZ(w) = wsize;
  }
}

#else

void _mpz_modadd (mpz_ptr w, mpz_srcptr u, mpz_srcptr v, mpz_srcptr p);
void _mpz_modsub (mpz_ptr w, mpz_srcptr u, mpz_srcptr v, mpz_srcptr p);

#endif

#ifdef __cplusplus
}
#endif

#endif // _EC_MPZMOD_H_INCLUDED_
