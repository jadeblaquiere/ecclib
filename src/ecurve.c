//BSD 3-Clause License
//
//Copyright (c) 2018, jadeblaquiere
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

#include <assert.h>
#include <ecc/ecurve.h>
#include <ecc/field.h>
#include <gmp.h>
#include <stdlib.h>
#include <string.h>

static int _known_curve_type(mpECurve_t cv) {
    return (cv->type == EQTypeShortWeierstrass) || 
        (cv->type == EQTypeEdwards) ||
        (cv->type == EQTypeMontgomery) ||
        (cv->type == EQTypeTwistedEdwards);
}

typedef struct {
    _mpECurve_eq_type type;
    char *name;
    char *p;
    char *a;
    char *b;
    char *n;
    char *h;
    char *Gx;
    char *Gy;
    int bits;
} _std_ws_curve_t;

typedef struct {
    _mpECurve_eq_type type;
    char *name;
    char *p;
    char *c;
    char *d;
    char *n;
    char *h;
    char *Gx;
    char *Gy;
    int bits;
} _std_ed_curve_t;

typedef struct {
    _mpECurve_eq_type type;
    char *name;
    char *p;
    char *B;
    char *A;
    char *n;
    char *h;
    char *Gx;
    char *Gy;
    int bits;
} _std_mo_curve_t;

typedef struct {
    _mpECurve_eq_type type;
    char *name;
    char *p;
    char *a;
    char *d;
    char *n;
    char *h;
    char *Gx;
    char *Gy;
    int bits;
} _std_te_curve_t;

typedef union {
    _std_ws_curve_t ws;
    _std_ed_curve_t ed;
    _std_mo_curve_t mo;
    _std_te_curve_t te;
} _std_curve_t;

// curves from http://www.secg.org/collateral/sec2_final.pdf
// also http://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.186-4.pdf
// also http://www.ecc-brainpool.org/download/Domain-parameters.pdf

static _std_ws_curve_t _std_ws_curve[] = {
    {
        EQTypeShortWeierstrass,
        "secp112r1",
        "0xDB7C2ABF62E35E668076BEAD208B",
        "0xDB7C2ABF62E35E668076BEAD2088",
        "0x659EF8BA043916EEDE8911702B22",
        "0xDB7C2ABF62E35E7628DFAC6561C5",
        "1",
        "0x09487239995A5EE76B55F9C2F098",
        "0xA89CE5AF8724C0A23E0E0FF77500",
        112
    },
    {
        EQTypeShortWeierstrass,
        "secp112r2",
        "0xDB7C2ABF62E35E668076BEAD208B",
        "0x6127C24C05F38A0AAAF65C0EF02C",
        "0x51DEF1815DB5ED74FCC34C85D709",
        "0x36DF0AAFD8B8D7597CA10520D04B",
        "4",
        "0x4BA30AB5E892B4E1649DD0928643",
        "0xADCD46F5882E3747DEF36E956E97",
        112
    },
    {
        EQTypeShortWeierstrass,
        "secp128r1",
        "0xFFFFFFFDFFFFFFFFFFFFFFFFFFFFFFFF",
        "0xFFFFFFFDFFFFFFFFFFFFFFFFFFFFFFFC",
        "0xE87579C11079F43DD824993C2CEE5ED3",
        "0xFFFFFFFE0000000075A30D1B9038A115",
        "1",
        "0x161FF7528B899B2D0C28607CA52C5B86",
        "0xCF5AC8395BAFEB13C02DA292DDED7A83",
        128
    },
    {
        EQTypeShortWeierstrass,
        "secp128r2",
        "0xFFFFFFFDFFFFFFFFFFFFFFFFFFFFFFFF",
        "0xD6031998D1B3BBFEBF59CC9BBFF9AEE1",
        "0x5EEEFCA380D02919DC2C6558BB6D8A5D",
        "0x3FFFFFFF7FFFFFFFBE0024720613B5A3",
        "4",
        "0x7B6AA5D85E572983E6FB32A7CDEBC140",
        "0x27B6916A894D3AEE7106FE805FC34B44",
        128
    },
    {
        EQTypeShortWeierstrass,
        "secp160k1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFAC73",
        "0",
        "7",
        "0x0100000000000000000001B8FA16DFAB9ACA16B6B3",
        "1",
        "0x3B4C382CE37AA192A4019E763036F4F5DD4D7EBB",
        "0x938CF935318FDCED6BC28286531733C3F03C4FEE",
        160
    },
    {
        EQTypeShortWeierstrass,
        "secp160r1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7FFFFFFF",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7FFFFFFC",
        "0x1C97BEFC54BD7A8B65ACF89F81D4D4ADC565FA45",
        "0x0100000000000000000001F4C8F927AED3CA752257",
        "1",
        "0x4A96B5688EF573284664698968C38BB913CBFC82",
        "0x23A628553168947D59DCC912042351377AC5FB32",
        160
    },
    {
        EQTypeShortWeierstrass,
        "secp160r2",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFAC73",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFAC70",
        "0xB4E134D3FB59EB8BAB57274904664D5AF50388BA",
        "0x0100000000000000000000351EE786A818F3A1A16B",
        "1",
        "0x52DCB034293A117E1F4FF11B30F7199D3144CE6D",
        "0xFEAFFEF2E331F296E071FA0DF9982CFEA7D43F2E",
        160
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP160r1",
        "0xE95E4A5F737059DC60DFC7AD95B3D8139515620F",
        "0x340E7BE2A280EB74E2BE61BADA745D97E8F7C300",
        "0x1E589A8595423412134FAA2DBDEC95C8D8675E58",
        "0xE95E4A5F737059DC60DF5991D45029409E60FC09",
        "1",
        "0xBED5AF16EA3F6A4F62938C4631EB5AF7BDBCDBC3",
        "0x1667CB477A1A8EC338F94741669C976316DA6321",
        160
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP160t1",
        "0xE95E4A5F737059DC60DFC7AD95B3D8139515620F",
        "0xE95E4A5F737059DC60DFC7AD95B3D8139515620C",
        "0x7A556B6DAE535B7B51ED2C4D7DAA7A0B5C55F380",
        "0xE95E4A5F737059DC60DF5991D45029409E60FC09",
        "1",
        "0xB199B13B9B34EFC1397E64BAEB05ACC265FF2378",
        "0xADD6718B7C7C1961F0991B842443772152C9E0AD",
        160
    },
    {
        EQTypeShortWeierstrass,
        "secp192k1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFEE37",
        "0",
        "3",
        "0xFFFFFFFFFFFFFFFFFFFFFFFE26F2FC170F69466A74DEFD8D",
        "1",
        "0xDB4FF10EC057E9AE26B07D0280B7F4341DA5D1B1EAE06C7D",
        "0x9B2F2F6D9C5628A7844163D015BE86344082AA88D95E2F9D",
        192
    },
    {
        EQTypeShortWeierstrass,
        "secp192r1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFF",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFC",
        "0x64210519E59C80E70FA7E9AB72243049FEB8DEECC146B9B1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22831",
        "1",
        "0x188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012",
        "0x07192B95FFC8DA78631011ED6B24CDD573F977A11E794811",
        192
    },
    // secp192r1 is also known as P192 FIPS 186-4 curve set
    {
        EQTypeShortWeierstrass,
        "P192",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFF",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFC",
        "0x64210519E59C80E70FA7E9AB72243049FEB8DEECC146B9B1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22831",
        "1",
        "0x188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012",
        "0x07192B95FFC8DA78631011ED6B24CDD573F977A11E794811",
        192
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP192r1",
        "0xC302F41D932A36CDA7A3463093D18DB78FCE476DE1A86297",
        "0x6A91174076B1E0E19C39C031FE8685C1CAE040E5C69A28EF",
        "0x469A28EF7C28CCA3DC721D044F4496BCCA7EF4146FBF25C9",
        "0xC302F41D932A36CDA7A3462F9E9E916B5BE8F1029AC4ACC1",
        "1",
        "0xC0A0647EAAB6A48753B033C56CB0F0900A2F5C4853375FD6",
        "0x14B690866ABD5BB88B5F4828C1490002E6773FA2FA299B8F",
        192
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP192t1",
        "0xC302F41D932A36CDA7A3463093D18DB78FCE476DE1A86297",
        "0xC302F41D932A36CDA7A3463093D18DB78FCE476DE1A86294",
        "0x13D56FFAEC78681E68F9DEB43B35BEC2FB68542E27897B79",
        "0xC302F41D932A36CDA7A3462F9E9E916B5BE8F1029AC4ACC1",
        "1",
        "0x3AE9E58C82F63C30282E1FE7BBF43FA72C446AF6F4618129",
        "0x097E2C5667C2223A902AB5CA449D0084B7E5B3DE7CCC01C9",
        192
    },
    {
        EQTypeShortWeierstrass,
        "secp224k1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFE56D",
        "0",
        "5",
        "0x010000000000000000000000000001DCE8D2EC6184CAF0A971769FB1F7",
        "1",
        "0xA1455B334DF099DF30FC28A169A467E9E47075A90F7E650EB6B7A45C",
        "0x7E089FED7FBA344282CAFBD6F7E319F7C0B0BD59E2CA4BDB556D61A5",
        224
    },
    {
        EQTypeShortWeierstrass,
        "secp224r1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFE",
        "0xB4050A850C04B3ABF54132565044B0B7D7BFD8BA270B39432355FFB4",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFF16A2E0B8F03E13DD29455C5C2A3D",
        "1",
        "0xB70E0CBD6BB4BF7F321390B94A03C1D356C21122343280D6115C1D21",
        "0xBD376388B5F723FB4C22DFE6CD4375A05A07476444D5819985007E34",
        224
    },
    // secp224r1 is also known as P224 FIPS 186-4 curve set
    {
        EQTypeShortWeierstrass,
        "P224",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFE",
        "0xB4050A850C04B3ABF54132565044B0B7D7BFD8BA270B39432355FFB4",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFF16A2E0B8F03E13DD29455C5C2A3D",
        "1",
        "0xB70E0CBD6BB4BF7F321390B94A03C1D356C21122343280D6115C1D21",
        "0xBD376388B5F723FB4C22DFE6CD4375A05A07476444D5819985007E34",
        224
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP224r1",
        "0xD7C134AA264366862A18302575D1D787B09F075797DA89F57EC8C0FF",
        "0x68A5E62CA9CE6C1C299803A6C1530B514E182AD8B0042A59CAD29F43",
        "0x2580F63CCFE44138870713B1A92369E33E2135D266DBB372386C400B",
        "0xD7C134AA264366862A18302575D0FB98D116BC4B6DDEBCA3A5A7939F",
        "1",
        "0x0D9029AD2C7E5CF4340823B2A87DC68C9E4CE3174C1E6EFDEE12C07D",
        "0x58AA56F772C0726F24C6B89E4ECDAC24354B9E99CAA3F6D3761402CD",
        224
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP224t1",
        "0xD7C134AA264366862A18302575D1D787B09F075797DA89F57EC8C0FF",
        "0xD7C134AA264366862A18302575D1D787B09F075797DA89F57EC8C0FC",
        "0x4B337D934104CD7BEF271BF60CED1ED20DA14C08B3BB64F18A60888D",
        "0xD7C134AA264366862A18302575D0FB98D116BC4B6DDEBCA3A5A7939F",
        "1",
        "0x6AB1E344CE25FF3896424E7FFE14762ECB49F8928AC0C76029B4D580",
        "0x374E9F5143E568CD23F3F4D7C0D4B1E41C8CC0D1C6ABD5F1A46DB4C",
        224
    },
    {
        EQTypeShortWeierstrass,
        "secp256k1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F",
        "0",
        "7",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141",
        "1",
        "0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798",
        "0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8",
        256
    },
    {
        EQTypeShortWeierstrass,
        "secp256r1",
        "0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF",
        "0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC",
        "0x5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B",
        "0xFFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551",
        "1",
        "0x6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296",
        "0x4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5",
        256
    },
    // secp256r1 is also known as P256 FIPS 186-4 curve set
    {
        EQTypeShortWeierstrass,
        "P256",
        "0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF",
        "0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC",
        "0x5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B",
        "0xFFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551",
        "1",
        "0x6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296",
        "0x4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5",
        256
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP256r1",
        "0xA9FB57DBA1EEA9BC3E660A909D838D726E3BF623D52620282013481D1F6E5377",
        "0x7D5A0975FC2C3057EEF67530417AFFE7FB8055C126DC5C6CE94A4B44F330B5D9",
        "0x26DC5C6CE94A4B44F330B5D9BBD77CBF958416295CF7E1CE6BCCDC18FF8C07B6",
        "0xA9FB57DBA1EEA9BC3E660A909D838D718C397AA3B561A6F7901E0E82974856A7",
        "1",
        "0x8BD2AEB9CB7E57CB2C4B482FFC81B7AFB9DE27E1E3BD23C23A4453BD9ACE3262",
        "0x547EF835C3DAC4FD97F8461A14611DC9C27745132DED8E545C1D54C72F046997",
        256
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP256t1",
        "0xA9FB57DBA1EEA9BC3E660A909D838D726E3BF623D52620282013481D1F6E5377",
        "0xA9FB57DBA1EEA9BC3E660A909D838D726E3BF623D52620282013481D1F6E5374",
        "0x662C61C430D84EA4FE66A7733D0B76B7BF93EBC4AF2F49256AE58101FEE92B04",
        "0xA9FB57DBA1EEA9BC3E660A909D838D718C397AA3B561A6F7901E0E82974856A7",
        "1",
        "0xA3E8EB3CC1CFE7B7732213B23A656149AFA142C47AAFBC2B79A191562E1305F4",
        "0x2D996C823439C56D7F7B22E14644417E69BCB6DE39D027001DABE8F35B25C9BE",
        256
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP320r1",
        "0xD35E472036BC4FB7E13C785ED201E065F98FCFA6F6F40DEF4F92B9EC7893EC28FCD412B1F1B32E27",
        "0x3EE30B568FBAB0F883CCEBD46D3F3BB8A2A73513F5EB79DA66190EB085FFA9F492F375A97D860EB4",
        "0x520883949DFDBC42D3AD198640688A6FE13F41349554B49ACC31DCCD884539816F5EB4AC8FB1F1A6",
        "0xD35E472036BC4FB7E13C785ED201E065F98FCFA5B68F12A32D482EC7EE8658E98691555B44C59311",
        "1",
        "0x43BD7E9AFB53D8B85289BCC48EE5BFE6F20137D10A087EB6E7871E2A10A599C710AF8D0D39E20611",
        "0x14FDD05545EC1CC8AB4093247F77275E0743FFED117182EAA9C77877AAAC6AC7D35245D1692E8EE1",
        320
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP320t1",
        "0xD35E472036BC4FB7E13C785ED201E065F98FCFA6F6F40DEF4F92B9EC7893EC28FCD412B1F1B32E27",
        "0xD35E472036BC4FB7E13C785ED201E065F98FCFA6F6F40DEF4F92B9EC7893EC28FCD412B1F1B32E24",
        "0xA7F561E038EB1ED560B3D147DB782013064C19F27ED27C6780AAF77FB8A547CEB5B4FEF422340353",
        "0xD35E472036BC4FB7E13C785ED201E065F98FCFA5B68F12A32D482EC7EE8658E98691555B44C59311",
        "1",
        "0x925BE9FB01AFC6FB4D3E7D4990010F813408AB106C4F09CB7EE07868CC136FFF3357F624A21BED52",
        "0x63BA3A7A27483EBF6671DBEF7ABB30EBEE084E58A0B077AD42A5A0989D1EE71B1B9BC0455FB0D2C3",
        320
    },
    {
        EQTypeShortWeierstrass,
        "secp384r1",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFF",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFC",
        "0xB3312FA7E23EE7E4988E056BE3F82D19181D9C6EFE8141120314088F5013875AC656398D8A2ED19D2A85C8EDD3EC2AEF",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC7634D81F4372DDF581A0DB248B0A77AECEC196ACCC52973",
        "1",
        "0xAA87CA22BE8B05378EB1C71EF320AD746E1D3B628BA79B9859F741E082542A385502F25DBF55296C3A545E3872760AB7",
        "0x3617DE4A96262C6F5D9E98BF9292DC29F8F41DBD289A147CE9DA3113B5F0B8C00A60B1CE1D7E819D7A431D7C90EA0E5F",
        384
    },
    // secp384r1 is also known as P384 FIPS 186-4 curve set
    {
        EQTypeShortWeierstrass,
        "P384",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFF",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFC",
        "0xB3312FA7E23EE7E4988E056BE3F82D19181D9C6EFE8141120314088F5013875AC656398D8A2ED19D2A85C8EDD3EC2AEF",
        "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC7634D81F4372DDF581A0DB248B0A77AECEC196ACCC52973",
        "1",
        "0xAA87CA22BE8B05378EB1C71EF320AD746E1D3B628BA79B9859F741E082542A385502F25DBF55296C3A545E3872760AB7",
        "0x3617DE4A96262C6F5D9E98BF9292DC29F8F41DBD289A147CE9DA3113B5F0B8C00A60B1CE1D7E819D7A431D7C90EA0E5F",
        384
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP384r1",
        "0x8CB91E82A3386D280F5D6F7E50E641DF152F7109ED5456B412B1DA197FB71123ACD3A729901D1A71874700133107EC53",
        "0x7BC382C63D8C150C3C72080ACE05AFA0C2BEA28E4FB22787139165EFBA91F90F8AA5814A503AD4EB04A8C7DD22CE2826",
        "0x04A8C7DD22CE28268B39B55416F0447C2FB77DE107DCD2A62E880EA53EEB62D57CB4390295DBC9943AB78696FA504C11",
        "0x8CB91E82A3386D280F5D6F7E50E641DF152F7109ED5456B31F166E6CAC0425A7CF3AB6AF6B7FC3103B883202E9046565",
        "1",
        "0x1D1C64F068CF45FFA2A63A81B7C13F6B8847A3E77EF14FE3DB7FCAFE0CBD10E8E826E03436D646AAEF87B2E247D4AF1E",
        "0x8ABE1D7520F9C2A45CB1EB8E95CFD55262B70B29FEEC5864E19C054FF99129280E4646217791811142820341263C5315",
        384
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP384t1",
        "0x8CB91E82A3386D280F5D6F7E50E641DF152F7109ED5456B412B1DA197FB71123ACD3A729901D1A71874700133107EC53",
        "0x8CB91E82A3386D280F5D6F7E50E641DF152F7109ED5456B412B1DA197FB71123ACD3A729901D1A71874700133107EC50",
        "0x7F519EADA7BDA81BD826DBA647910F8C4B9346ED8CCDC64E4B1ABD11756DCE1D2074AA263B88805CED70355A33B471EE",
        "0x8CB91E82A3386D280F5D6F7E50E641DF152F7109ED5456B31F166E6CAC0425A7CF3AB6AF6B7FC3103B883202E9046565",
        "1",
        "0x18DE98B02DB9A306F2AFCD7235F72A819B80AB12EBD653172476FECD462AABFFC4FF191B946A5F54D8D0AA2F418808CC",
        "0x25AB056962D30651A114AFD2755AD336747F93475B7A1FCA3B88F2B6A208CCFE469408584DC2B2912675BF5B9E582928",
        384
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP512r1",
        "0xAADD9DB8DBE9C48B3FD4E6AE33C9FC07CB308DB3B3C9D20ED6639CCA703308717D4D9B009BC66842AECDA12AE6A380E62881FF2F2D82C68528AA6056583A48F3",
        "0x7830A3318B603B89E2327145AC234CC594CBDD8D3DF91610A83441CAEA9863BC2DED5D5AA8253AA10A2EF1C98B9AC8B57F1117A72BF2C7B9E7C1AC4D77FC94CA",
        "0x3DF91610A83441CAEA9863BC2DED5D5AA8253AA10A2EF1C98B9AC8B57F1117A72BF2C7B9E7C1AC4D77FC94CADC083E67984050B75EBAE5DD2809BD638016F723",
        "0xAADD9DB8DBE9C48B3FD4E6AE33C9FC07CB308DB3B3C9D20ED6639CCA70330870553E5C414CA92619418661197FAC10471DB1D381085DDADDB58796829CA90069",
        "1",
        "0x81AEE4BDD82ED9645A21322E9C4C6A9385ED9F70B5D916C1B43B62EEF4D0098EFF3B1F78E2D0D48D50D1687B93B97D5F7C6D5047406A5E688B352209BCB9F822",
        "0x7DDE385D566332ECC0EABFA9CF7822FDF209F70024A57B1AA000C55B881F8111B2DCDE494A5F485E5BCA4BD88A2763AED1CA2B2FA8F0540678CD1E0F3AD80892",
        512
    },
    {
        EQTypeShortWeierstrass,
        "brainpoolP512t1",
        "0xAADD9DB8DBE9C48B3FD4E6AE33C9FC07CB308DB3B3C9D20ED6639CCA703308717D4D9B009BC66842AECDA12AE6A380E62881FF2F2D82C68528AA6056583A48F3",
        "0xAADD9DB8DBE9C48B3FD4E6AE33C9FC07CB308DB3B3C9D20ED6639CCA703308717D4D9B009BC66842AECDA12AE6A380E62881FF2F2D82C68528AA6056583A48F0",
        "0x7CBBBCF9441CFAB76E1890E46884EAE321F70C0BCB4981527897504BEC3E36A62BCDFA2304976540F6450085F2DAE145C22553B465763689180EA2571867423E",
        "0xAADD9DB8DBE9C48B3FD4E6AE33C9FC07CB308DB3B3C9D20ED6639CCA70330870553E5C414CA92619418661197FAC10471DB1D381085DDADDB58796829CA90069",
        "1",
        "0x640ECE5C12788717B9C1BA06CBC2A6FEBA85842458C56DDE9DB1758D39C0313D82BA51735CDB3EA499AA77A7D6943A64F7A3F25FE26F06B51BAA2696FA9035DA",
        "0x5B534BD595F5AF0FA2C892376C84ACE1BB4E3019B71634C01131159CAE03CEE9D9932184BEEF216BD71DF2DADF86A627306ECFF96DBB8BACE198B61E00F8B332",
        512
    },
    {
        EQTypeShortWeierstrass,
        "secp521r1",
        "0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
        "0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC",
        "0x0051953EB9618E1C9A1F929A21A0B68540EEA2DA725B99B315F3B8B489918EF109E156193951EC7E937B1652C0BD3BB1BF073573DF883D2C34F1EF451FD46B503F00",
        "0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFA51868783BF2F966B7FCC0148F709A5D03BB5C9B8899C47AEBB6FB71E91386409",
        "1",
        "0x00C6858E06B70404E9CD9E3ECB662395B4429C648139053FB521F828AF606B4D3DBAA14B5E77EFE75928FE1DC127A2FFA8DE3348B3C1856A429BF97E7E31C2E5BD66",
        "0x011839296A789A3BC0045C8A5FB42C7D1BD998F54449579B446817AFBD17273E662C97EE72995EF42640C550B9013FAD0761353C7086A272C24088BE94769FD16650",
        521
    },
    // secp521r1 is also known as P521 FIPS 186-4 curve set
    {
        EQTypeShortWeierstrass,
        "P521",
        "0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
        "0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC",
        "0x0051953EB9618E1C9A1F929A21A0B68540EEA2DA725B99B315F3B8B489918EF109E156193951EC7E937B1652C0BD3BB1BF073573DF883D2C34F1EF451FD46B503F00",
        "0x01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFA51868783BF2F966B7FCC0148F709A5D03BB5C9B8899C47AEBB6FB71E91386409",
        "1",
        "0x00C6858E06B70404E9CD9E3ECB662395B4429C648139053FB521F828AF606B4D3DBAA14B5E77EFE75928FE1DC127A2FFA8DE3348B3C1856A429BF97E7E31C2E5BD66",
        "0x011839296A789A3BC0045C8A5FB42C7D1BD998F54449579B446817AFBD17273E662C97EE72995EF42640C550B9013FAD0761353C7086A272C24088BE94769FD16650",
        521
    }
};

// Edwards curves from https://safecurves.cr.yp.to/ and 
// https://ed25519.cr.yp.to/software.html

static _std_ed_curve_t _std_ed_curve[] = {
    {
        EQTypeEdwards,
        "E-222",
        "0x3fffffffffffffffffffffffffffffffffffffffffffffffffffff8b",
        "1",
        "160102",
        "0xffffffffffffffffffffffffffff70cbc95e932f802f31423598cbf",
        "4",
        "0x19b12bb156a389e55c9768c303316d07c23adab3736eb2bc3eb54e51",
        "0x1c",
        222
    },
    {
        EQTypeEdwards,
        "Curve1174",
        "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7",
        "1",
        // p - 1174 = 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffb61
        "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffb61",
        "0x1fffffffffffffffffffffffffffffff77965c4dfd307348944d45fd166c971",
        "4",
        "0x37fbb0cea308c479343aee7c029a190c021d96a492ecd6516123f27bce29eda",
        "0x6b72f82d47fb7cc6656841169840e0c4fe2dee2af3f976ba4ccb1bf9b46360e",
        251
    },
    {
        EQTypeEdwards,
        "E-382",
        "0x3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff97",
        "1",
        // p - 67254 = 0x3ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef8e1
        "0x3ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef8e1",
        "0xfffffffffffffffffffffffffffffffffffffffffffffffd5fb21f21e95eee17c5e69281b102d2773e27e13fd3c9719",
        "4",
        "0x196f8dd0eab20391e5f05be96e8d20ae68f840032b0b64352923bab85364841193517dbce8105398ebc0cc9470f79603",
        "0x11",
        382
    },
    {
        EQTypeEdwards,
        "Curve41417",
        "0x3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef",
        "1",
        "3617",
        "0x7ffffffffffffffffffffffffffffffffffffffffffffffffffeb3cc92414cf706022b36f1c0338ad63cf181b0e71a5e106af79",
        "8",
        "0x1a334905141443300218c0631c326e5fcd46369f44c03ec7f57ff35498a4ab4d6d6ba111301a73faa8537c64c4fd3812f3cbc595",
        "0x22",
        414
    },
    {
        EQTypeEdwards,
        "Ed448-Goldilocks",
        "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffeffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
        "1",
        // p - 39081 = 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffeffffffffffffffffffffffffffffffffffffffffffffffffffff6756
        "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffeffffffffffffffffffffffffffffffffffffffffffffffffffff6756",
        "0x3fffffffffffffffffffffffffffffffffffffffffffffffffffffff7cca23e9c44edb49aed63690216cc2728dc58f552378c292ab5844f3",
        "4",
        "0x297ea0ea2692ff1b4faff46098453a6a26adf733245f065c3c59d0709cecfa96147eaaf3932d94c63d96c170033f4ba0c7f0de840aed939f",
        "0x13",
        448
    },
    {
        EQTypeEdwards,
        "E-521",
        "0x1ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
        "1",
        // p - 376014 = 0x1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa4331
        "0x1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa4331",
        "0x7ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd15b6c64746fc85f736b8af5e7ec53f04fbd8c4569a8f1f4540ea2435f5180d6b",
        "4",
        "0x752cb45c48648b189df90cb2296b2878a3bfd9f42fc6c818ec8bf3c9c0c6203913f6ecc5ccc72434b1ae949d568fc99c6059d0fb13364838aa302a940a2f19ba6c",
        "0xc",
        521
    }
};

// Montgomery curves from https://safecurves.cr.yp.to/

static _std_mo_curve_t _std_mo_curve[] = {
    {
        EQTypeMontgomery,
        "M-221",
        "0x1ffffffffffffffffffffffffffffffffffffffffffffffffffffffd",
        "1",
        "117050",
        "0x40000000000000000000000000015a08ed730e8a2f77f005042605b",
        "8",
        "0x4",
        "0xf7acdd2a4939571d1cef14eca37c228e61dbff10707dc6c08c5056d",
        221
    },
    {
        EQTypeMontgomery,
        "Curve25519",
        "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed",
        "1",
        "486662",
        "0x1000000000000000000000000000000014def9dea2f79cd65812631a5cf5d3ed",
        "8",
        "0x9",
        "0x20ae19a1b8a086b4e01edd2c7748d14c923d4d7e6d7c61b229e9c5a27eced3d9",
        255
    },
    {
        EQTypeMontgomery,
        "M-383",
        "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff45",
        "1",
        "2065150",
        "0x10000000000000000000000000000000000000000000000006c79673ac36ba6e7a32576f7b1b249e46bbc225be9071d7",
        "8",
        "0xc",
        "0x1ec7ed04aaf834af310e304b2da0f328e7c165f0e8988abd3992861290f617aa1f1b2e7d0b6e332e969991b62555e77e",
        383
    },
    {
        EQTypeMontgomery,
        "Curve383187",
        "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff45",
        "1",
        "229969",
        "0x1000000000000000000000000000000000000000000000000e85a85287a1488acd41ae84b2b7030446f72088b00a0e21",
        "8",
        "0x5",
        "0x1eebe07dc1871896732b12d5504a32370471965c7a11f2c89865f855ab3cbd7c224e3620c31af3370788457dd5ce46df",
        383
    },
    {
        EQTypeMontgomery,
        "M-511",
        "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff45",
        "1",
        "530438",
        "0x100000000000000000000000000000000000000000000000000000000000000017b5feff30c7f5677ab2aeebd13779a2ac125042a6aa10bfa54c15bab76baf1b",
        "8",
        "0x5",
        "0x2fbdc0ad8530803d28fdbad354bb488d32399ac1cf8f6e01ee3f96389b90c809422b9429e8a43dbf49308ac4455940abe9f1dbca542093a895e30a64af056fa5",
        511
    }
};

static _std_te_curve_t _std_te_curve[] = {
    {
        EQTypeTwistedEdwards,
        "Ed25519",
        "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED",
        "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEC",
        "0x52036CEE2B6FFE738CC740797779E89800700A4D4141D8AB75EB4DCA135978A3",
        "0x1000000000000000000000000000000014DEF9DEA2F79CD65812631A5CF5D3ED",
        "8",
        "0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A",
        "0x6666666666666666666666666666666666666666666666666666666666666658",
        255
    }
};

static void _mpECurve_init_coeff(mpECurve_t cv) {
    switch (cv->type) {
        case EQTypeShortWeierstrass:
            assert(cv->fp != NULL);
            mpFp_init_fp(cv->coeff.ws.a, cv->fp);
            mpFp_init_fp(cv->coeff.ws.b, cv->fp);
            break;
        case EQTypeEdwards:
            assert(cv->fp != NULL);
            mpFp_init_fp(cv->coeff.ed.c, cv->fp);
            mpFp_init_fp(cv->coeff.ed.d, cv->fp);
            break;
        case EQTypeMontgomery:
            assert(cv->fp != NULL);
            mpFp_init_fp(cv->coeff.mo.B, cv->fp);
            mpFp_init_fp(cv->coeff.mo.A, cv->fp);
            mpFp_init_fp(cv->coeff.mo.ws_a, cv->fp);
            mpFp_init_fp(cv->coeff.mo.ws_b, cv->fp);
            mpFp_init_fp(cv->coeff.mo.Binv, cv->fp);
            mpFp_init_fp(cv->coeff.mo.Adiv3, cv->fp);
            break;
        case EQTypeTwistedEdwards:
            assert(cv->fp != NULL);
            mpFp_init_fp(cv->coeff.te.a, cv->fp);
            mpFp_init_fp(cv->coeff.te.d, cv->fp);
            break;
        case EQTypeUninitialized:
            break;
        default:
            assert(_known_curve_type(cv));
    }
    return;
}

static void _mpECurve_clear_coeff(mpECurve_t cv) {
    switch (cv->type) {
        case EQTypeShortWeierstrass:
            mpFp_clear(cv->coeff.ws.a);
            mpFp_clear(cv->coeff.ws.b);
            break;
        case EQTypeEdwards:
            mpFp_clear(cv->coeff.ed.c);
            mpFp_clear(cv->coeff.ed.d);
            break;
        case EQTypeMontgomery:
            mpFp_clear(cv->coeff.mo.B);
            mpFp_clear(cv->coeff.mo.A);
            mpFp_clear(cv->coeff.mo.ws_a);
            mpFp_clear(cv->coeff.mo.ws_b);
            mpFp_clear(cv->coeff.mo.Binv);
            mpFp_clear(cv->coeff.mo.Adiv3);
            break;
        case EQTypeTwistedEdwards:
            mpFp_clear(cv->coeff.te.a);
            mpFp_clear(cv->coeff.te.d);
            break;
        case EQTypeUninitialized:
            break;
        default:
            assert(_known_curve_type(cv));
    }
    cv->type = EQTypeUninitialized;
    return;
}

void mpECurve_init(mpECurve_t c) {
    // default type is short Weierstrass
    c->type = EQTypeUninitialized;
    //mpz_init(c->p);
    //mpz_set(c->p, p);
    c->fp = NULL;
    _mpECurve_init_coeff(c);
    mpz_init(c->n);
    mpz_init(c->h);
    mpz_init(c->G[0]);
    mpz_init(c->G[1]);
    c->bits = 0;
    return;
}

void mpECurve_clear(mpECurve_t c) {
    _mpECurve_clear_coeff(c);
    c->fp = NULL;
    mpz_clear(c->n);
    mpz_clear(c->h);
    mpz_clear(c->G[0]);
    mpz_clear(c->G[1]);
#ifdef  SAFE_CLEAN
    memset((void *)c, 0, sizeof(*c));
#endif
    return;
}

void mpECurve_set(mpECurve_t rop, mpECurve_t op){
    rop->fp = op->fp;
    if (rop->type != op->type) {
        _mpECurve_clear_coeff(rop);
        rop->type = op->type;
        _mpECurve_init_coeff(rop);
    }
    switch (op->type) {
        case EQTypeShortWeierstrass:
            mpFp_set(rop->coeff.ws.a, op->coeff.ws.a);
            mpFp_set(rop->coeff.ws.b, op->coeff.ws.b);
            break;
        case EQTypeEdwards:
            mpFp_set(rop->coeff.ed.c, op->coeff.ed.c);
            mpFp_set(rop->coeff.ed.d, op->coeff.ed.d);
            break;
        case EQTypeMontgomery:
            mpFp_set(rop->coeff.mo.B, op->coeff.mo.B);
            mpFp_set(rop->coeff.mo.A, op->coeff.mo.A);
            mpFp_set(rop->coeff.mo.ws_a, op->coeff.mo.ws_a);
            mpFp_set(rop->coeff.mo.ws_b, op->coeff.mo.ws_b);
            mpFp_set(rop->coeff.mo.Binv, op->coeff.mo.Binv);
            mpFp_set(rop->coeff.mo.Adiv3, op->coeff.mo.Adiv3);
            break;
        case EQTypeTwistedEdwards:
            mpFp_set(rop->coeff.te.a, op->coeff.te.a);
            mpFp_set(rop->coeff.te.d, op->coeff.te.d);
            break;
        default:
            assert(_known_curve_type(op));
    }
    mpz_set(rop->n, op->n);
    mpz_set(rop->h, op->h);
    mpz_set(rop->G[0], op->G[0]);
    mpz_set(rop->G[1], op->G[1]);
    rop->bits = op->bits;
    return;
}

int mpECurve_set_str_ws(mpECurve_t cv, char *p, char *a, char *b, char *n,
char *h, char *Gx, char *Gy, unsigned int bits){
    int status;
    mpFp_field_ptr fp;
    mpz_t t;

    mpz_init(t);
    mpz_set_str(t, p, 0);
    fp = _mpFp_field_lookup(t);
    cv->fp = fp;
    assert(fp != NULL);
    if (cv->type != EQTypeShortWeierstrass) {
        _mpECurve_clear_coeff(cv);
        cv->type = EQTypeShortWeierstrass;
        _mpECurve_init_coeff(cv);
    }
    mpz_set_str(t, a, 0);
    mpFp_set_mpz_fp(cv->coeff.ws.a, t, cv->fp);
    mpz_set_str(t, b, 0);
    mpFp_set_mpz_fp(cv->coeff.ws.b, t, cv->fp);
    mpz_set_str(cv->n, n, 0);
    mpz_set_str(cv->h, h, 0);
    mpz_set_str(cv->G[0], Gx, 0);
    mpz_set_str(cv->G[1], Gy, 0);
    cv->bits = bits;
    status = (mpECurve_point_check(cv, cv->G[0], cv->G[1]) == 0);
    mpz_clear(t);
    return status;
}

int mpECurve_set_str_ed(mpECurve_t cv, char *p, char *c, char *d, char *n,
char *h, char *Gx, char *Gy, unsigned int bits){
    int status;
    mpFp_field_ptr fp;
    mpz_t t;

    mpz_init(t);
    mpz_set_str(t, p, 0);
    fp = _mpFp_field_lookup(t);
    cv->fp = fp;
    if (cv->type != EQTypeEdwards) {
        _mpECurve_clear_coeff(cv);
        cv->type = EQTypeEdwards;
        _mpECurve_init_coeff(cv);
    }
    mpz_set_str(t, c, 0);
    mpFp_set_mpz_fp(cv->coeff.ed.c, t, cv->fp);
    mpz_set_str(t, d, 0);
    mpFp_set_mpz_fp(cv->coeff.ed.d, t, cv->fp);
    mpz_set_str(cv->n, n, 0);
    mpz_set_str(cv->h, h, 0);
    mpz_set_str(cv->G[0], Gx, 0);
    mpz_set_str(cv->G[1], Gy, 0);
    cv->bits = bits;
    status = (mpECurve_point_check(cv, cv->G[0], cv->G[1]) == 0);
    mpz_clear(t);
    return status;
}

int mpECurve_set_str_mo(mpECurve_t cv, char *p, char *B, char *A, char *n,
char *h, char *Gx, char *Gy, unsigned int bits){
    int status;
    mpz_t zp, zB, zA, zn, zh, zGx, zGy;

    mpz_init(zp);
    mpz_init(zB);
    mpz_init(zA);
    mpz_init(zn);
    mpz_init(zh);
    mpz_init(zGx);
    mpz_init(zGy);

    mpz_set_str(zp, p, 0);
    mpz_set_str(zB, B, 0);
    mpz_set_str(zA, A, 0);
    mpz_set_str(zn, n, 0);
    mpz_set_str(zh, h, 0);
    mpz_set_str(zGx, Gx, 0);
    mpz_set_str(zGy, Gy, 0);

    status = mpECurve_set_mpz_mo(cv, zp, zB, zA, zn, zh, zGx, zGy, bits);

    mpz_clear(zGy);
    mpz_clear(zGx);
    mpz_clear(zh);
    mpz_clear(zn);
    mpz_clear(zA);
    mpz_clear(zB);
    mpz_clear(zp);
    return status;
}

int mpECurve_set_str_te(mpECurve_t cv, char *p, char *a, char *d, char *n,
char *h, char *Gx, char *Gy, unsigned int bits){
    int status;
    mpFp_field_ptr fp;
    mpz_t t;

    mpz_init(t);
    mpz_set_str(t, p, 0);
    fp = _mpFp_field_lookup(t);
    cv->fp = fp;
    if (cv->type != EQTypeTwistedEdwards) {
        _mpECurve_clear_coeff(cv);
        cv->type = EQTypeTwistedEdwards;
        _mpECurve_init_coeff(cv);
    }
    mpz_set_str(t, a, 0);
    mpFp_set_mpz_fp(cv->coeff.te.a, t, cv->fp);
    mpz_set_str(t, d, 0);
    mpFp_set_mpz_fp(cv->coeff.te.d, t, cv->fp);
    mpz_set_str(cv->n, n, 0);
    mpz_set_str(cv->h, h, 0);
    mpz_set_str(cv->G[0], Gx, 0);
    mpz_set_str(cv->G[1], Gy, 0);
    cv->bits = bits;
    status = (mpECurve_point_check(cv, cv->G[0], cv->G[1]) == 0);
    mpz_clear(t);
    return status;
}

int mpECurve_set_mpz_ws(mpECurve_t cv, mpz_t p, mpz_t a, mpz_t b, mpz_t n,
mpz_t h, mpz_t Gx, mpz_t Gy, unsigned int bits){
    int status;
    mpFp_field_ptr fp;

    fp = _mpFp_field_lookup(p);
    cv->fp = fp;
    if (cv->type != EQTypeShortWeierstrass) {
        _mpECurve_clear_coeff(cv);
        cv->type = EQTypeShortWeierstrass;
        _mpECurve_init_coeff(cv);
    }
    mpFp_set_mpz_fp(cv->coeff.ws.a, a, cv->fp);
    mpFp_set_mpz_fp(cv->coeff.ws.b, b, cv->fp);
    mpz_set(cv->n, n);
    mpz_set(cv->h, h);
    mpz_set(cv->G[0], Gx);
    mpz_set(cv->G[1], Gy);
    cv->bits = bits;
    status = (mpECurve_point_check(cv, cv->G[0], cv->G[1]) == 0);
    return status;
}

int mpECurve_set_mpz_ed(mpECurve_t cv, mpz_t p, mpz_t c, mpz_t d, mpz_t n,
mpz_t h, mpz_t Gx, mpz_t Gy, unsigned int bits){
    int status;
    mpFp_field_ptr fp;

    fp = _mpFp_field_lookup(p);
    cv->fp = fp;
    if (cv->type != EQTypeEdwards) {
        _mpECurve_clear_coeff(cv);
        cv->type = EQTypeEdwards;
        _mpECurve_init_coeff(cv);
    }
    mpFp_set_mpz_fp(cv->coeff.ed.c, c, cv->fp);
    mpFp_set_mpz_fp(cv->coeff.ed.d, d, cv->fp);
    mpz_set(cv->n, n);
    mpz_set(cv->h, h);
    mpz_set(cv->G[0], Gx);
    mpz_set(cv->G[1], Gy);
    cv->bits = bits;
    status = (mpECurve_point_check(cv, cv->G[0], cv->G[1]) == 0);
    return status;
}

int mpECurve_set_mpz_mo(mpECurve_t cv, mpz_t p, mpz_t B, mpz_t A, mpz_t n,
mpz_t h, mpz_t Gx, mpz_t Gy, unsigned int bits){
    int status;
    mpFp_field_ptr fp;

    fp = _mpFp_field_lookup(p);
    _mpECurve_clear_coeff(cv);
    cv->fp = fp;
    cv->type = EQTypeMontgomery;
    _mpECurve_init_coeff(cv);
    mpFp_set_mpz_fp(cv->coeff.mo.B, B, cv->fp);
    mpFp_set_mpz_fp(cv->coeff.mo.A, A, cv->fp);
    // internal representation of Montgomery curve points is ws
    // to facilitate add/double (not differential)
    // transform is :
    // x = Bu-A/3, y = Bv
    // ws_a = (3-A^2)/(3B^2) and ws_b = (2A^3-9A)/(27B^3)
    // resulting equation:
    // v^2 = u^3 + ws_a * u + ws_b
    // reverse transform:
    // u = x/B + A/3, v = y/B
    {
        mpFp_t a, b, s, t;
        mpFp_init_fp(a, cv->fp);
        mpFp_init_fp(b, cv->fp);
        mpFp_init_fp(s, cv->fp);
        mpFp_init_fp(t, cv->fp);
        // precalculate A/3 and 1/B
        mpFp_set_ui_fp(t, 3, cv->fp);
        mpFp_inv(t, t);
        mpFp_mul(cv->coeff.mo.Adiv3, cv->coeff.mo.A, t);
        mpFp_inv(cv->coeff.mo.Binv, cv->coeff.mo.B);
        // calculate short Weierstrass curve coefficients
        // ws_a
        mpFp_mul(b, cv->coeff.mo.B, cv->coeff.mo.B);
        mpFp_mul_ui(s, b, 3);
        mpFp_inv(s, s);
        mpFp_mul(a, cv->coeff.mo.A, cv->coeff.mo.A);
        mpFp_set_ui_fp(t, 3, cv->fp);
        mpFp_sub(t, t, a);
        mpFp_mul(cv->coeff.mo.ws_a, t, s);
        // ws_b
        mpFp_mul(b, b, cv->coeff.mo.B);
        mpFp_mul_ui(b, b, 27);
        mpFp_inv(b, b);
        mpFp_mul(a, a, cv->coeff.mo.A);
        mpFp_mul_ui(a, a, 2);
        mpFp_mul_ui(s, cv->coeff.mo.A, 9);
        mpFp_sub(s, a, s);
        mpFp_mul(cv->coeff.mo.ws_b, s, b);
        mpFp_clear(t);
        mpFp_clear(s);
        mpFp_clear(b);
        mpFp_clear(a);
    }
    mpz_set(cv->n, n);
    mpz_set(cv->h, h);
    mpz_set(cv->G[0], Gx);
    mpz_set(cv->G[1], Gy);
    cv->bits = bits;
    status = (mpECurve_point_check(cv, cv->G[0], cv->G[1]) == 0);
    return status;
}

int mpECurve_set_mpz_te(mpECurve_t cv, mpz_t p, mpz_t a, mpz_t d, mpz_t n,
mpz_t h, mpz_t Gx, mpz_t Gy, unsigned int bits){
    int status;
    mpFp_field_ptr fp;

    fp = _mpFp_field_lookup(p);
    cv->fp = fp;
    if (cv->type != EQTypeTwistedEdwards) {
        _mpECurve_clear_coeff(cv);
        cv->type = EQTypeTwistedEdwards;
        _mpECurve_init_coeff(cv);
    }
    mpFp_set_mpz_fp(cv->coeff.te.a, a, cv->fp);
    mpFp_set_mpz_fp(cv->coeff.te.d, d, cv->fp);
    mpz_set(cv->n, n);
    mpz_set(cv->h, h);
    mpz_set(cv->G[0], Gx);
    mpz_set(cv->G[1], Gy);
    cv->bits = bits;
    status = (mpECurve_point_check(cv, cv->G[0], cv->G[1]) == 0);
    return status;
}

// note, assert used to check status here... since we're using a table
// lookup, if the curve parameters are in error this represents an internal
// error in the library
int mpECurve_set_named(mpECurve_t cv, char *name) {
    int i, ncurves, status;

    ncurves = sizeof(_std_ws_curve) / sizeof(_std_ws_curve[0]);
    for (i = 0 ; i < ncurves; i++) {
        if (strcmp(_std_ws_curve[i].name, name) == 0) {
            status = mpECurve_set_str_ws(cv, _std_ws_curve[i].p, _std_ws_curve[i].a,
                _std_ws_curve[i].b,_std_ws_curve[i].n,_std_ws_curve[i].h,
                _std_ws_curve[i].Gx,_std_ws_curve[i].Gy,_std_ws_curve[i].bits);
            assert (status == 0);
            return 0;
        }
    }
    ncurves = sizeof(_std_ed_curve) / sizeof(_std_ed_curve[0]);
    for (i = 0 ; i < ncurves; i++) {
        if (strcmp(_std_ed_curve[i].name, name) == 0) {
            status = mpECurve_set_str_ed(cv, _std_ed_curve[i].p, _std_ed_curve[i].c,
                _std_ed_curve[i].d,_std_ed_curve[i].n,_std_ed_curve[i].h,
                _std_ed_curve[i].Gx,_std_ed_curve[i].Gy,_std_ed_curve[i].bits);
            assert (status == 0);
            return 0;
        }
    }
    ncurves = sizeof(_std_mo_curve) / sizeof(_std_mo_curve[0]);
    for (i = 0 ; i < ncurves; i++) {
        if (strcmp(_std_mo_curve[i].name, name) == 0) {
            status = mpECurve_set_str_mo(cv, _std_mo_curve[i].p, _std_mo_curve[i].B,
                _std_mo_curve[i].A,_std_mo_curve[i].n,_std_mo_curve[i].h,
                _std_mo_curve[i].Gx,_std_mo_curve[i].Gy,_std_mo_curve[i].bits);
            assert (status == 0);
            return 0;
        }
    }
    ncurves = sizeof(_std_te_curve) / sizeof(_std_te_curve[0]);
    for (i = 0 ; i < ncurves; i++) {
        if (strcmp(_std_te_curve[i].name, name) == 0) {
            status = mpECurve_set_str_te(cv, _std_te_curve[i].p, _std_te_curve[i].a,
                _std_te_curve[i].d,_std_te_curve[i].n,_std_te_curve[i].h,
                _std_te_curve[i].Gx,_std_te_curve[i].Gy,_std_te_curve[i].bits);
            assert (status == 0);
            return 0;
        }
    }
    return -1;
}

int mpECurve_point_check(mpECurve_t cv, mpz_t Px, mpz_t Py) {
    mpFp_t x, y, r, l, t;
    int on_curve;
    mpFp_init_fp(x, cv->fp);
    mpFp_init_fp(y, cv->fp);
    mpFp_init_fp(r, cv->fp);
    mpFp_init_fp(l, cv->fp);
    mpFp_init_fp(t, cv->fp);

    mpFp_set_mpz_fp(x, Px, cv->fp);
    mpFp_set_mpz_fp(y, Py, cv->fp);

    // basic approach: calculate left and right sides and then compare l=r?

    switch (cv->type) {
        case EQTypeShortWeierstrass: {
            // y**2 = x**3 + ax + b
            mpFp_pow_ui(r, x, 3);
            mpFp_mul(l, cv->coeff.ws.a, x);
            mpFp_add(r, r, l);
            mpFp_add(r, r, cv->coeff.ws.b);
            mpFp_sqr(l, y);
            break;
        }
        case EQTypeEdwards: {
            // x**2 + y**2 = c**2 * (1 + (d * x**2 * y**2))
            mpFp_pow_ui(r, x, 2);
            mpFp_pow_ui(t, y, 2);
            mpFp_add(l, t, r);
            mpFp_mul(r, r, t);
            mpFp_mul(r, r, cv->coeff.ed.d);
            mpFp_add_ui(r, r, 1);
            mpFp_pow_ui(t, cv->coeff.ed.c, 2);
            mpFp_mul(r, r, t);
            break;
        }
        case EQTypeMontgomery: {
            // B * y**2 = x**3 + A * x**2 + x
            mpFp_pow_ui(l, y, 2);
            mpFp_mul(l, l, cv->coeff.mo.B);
            mpFp_pow_ui(r, x, 3);
            mpFp_add(r, r, x);
            mpFp_pow_ui(t, x, 2);
            mpFp_mul(t, t, cv->coeff.mo.A);
            mpFp_add(r, r, t);
            break;
        }
        case EQTypeTwistedEdwards: {
            // a * x**2 + y**2 = 1 + (d * x**2 * y**2)
            mpFp_pow_ui(l, x, 2);
            mpFp_pow_ui(t, y, 2);
            mpFp_mul(r, l, t);
            mpFp_mul(l, l, cv->coeff.te.a);
            mpFp_add(l, l, t);
            mpFp_mul(r, r, cv->coeff.te.d);
            mpFp_add_ui(r, r, 1);
            break;
        }
        default:
            assert(_known_curve_type(cv));
    }

    on_curve = (mpFp_cmp(l, r) == 0);
    mpFp_clear(t);
    mpFp_clear(l);
    mpFp_clear(r);
    mpFp_clear(y);
    mpFp_clear(x);
    return on_curve;
}

int mpECurve_cmp(mpECurve_t op1, mpECurve_t op2) {
    int r;
    if (op1 == op2) return 0;
    //printf("not the same pointer\n");
    if (op1->type != op2->type) return -1;
    //printf("same type\n");
    r = (op1->fp == op2->fp) ; if (r != 1) return r;
    //printf("same field\n");
    switch (op1->type) {
        case EQTypeShortWeierstrass:
            r = mpFp_cmp(op1->coeff.ws.a, op2->coeff.ws.a) ; if (r != 0) return r;
            //printf("ws: same a\n");
            r = mpFp_cmp(op1->coeff.ws.b, op2->coeff.ws.b) ; if (r != 0) return r;
            //printf("ws: same b\n");
            break;
        case EQTypeEdwards:
            r = mpFp_cmp(op1->coeff.ed.c, op2->coeff.ed.c) ; if (r != 0) return r;
            //printf("ed: same c\n");
            r = mpFp_cmp(op1->coeff.ed.d, op2->coeff.ed.d) ; if (r != 0) return r;
            //printf("ed: same d\n");
            break;
        case EQTypeMontgomery:
            r = mpFp_cmp(op1->coeff.mo.B, op2->coeff.mo.B) ; if (r != 0) return r;
            //printf("mo: same B\n");
            r = mpFp_cmp(op1->coeff.mo.A, op2->coeff.mo.A) ; if (r != 0) return r;
            //printf("mo: same A\n");
            break;
        case EQTypeTwistedEdwards:
            r = mpFp_cmp(op1->coeff.te.a, op2->coeff.te.a) ; if (r != 0) return r;
            //printf("te: same a\n");
            r = mpFp_cmp(op1->coeff.te.d, op2->coeff.te.d) ; if (r != 0) return r;
            //printf("te: same d\n");
            break;
        default:
            //printf("UNKNOWN CURVE TYPE???\n");
            assert(_known_curve_type(op1));
    }
    r = mpz_cmp(op1->n, op2->n) ; if (r != 0) return r;
    //printf("same n\n");
    r = mpz_cmp(op1->h, op2->h) ; if (r != 0) return r;
    //printf("same h\n");
    r = mpz_cmp(op1->G[0], op2->G[0]) ; if (r != 0) return r;
    //printf("same gx\n");
    r = mpz_cmp(op1->G[1], op2->G[1]) ; if (r != 0) return r;
    //printf("same gy\n");
    return 0;
}

char **_mpECurve_list_standard_curves() {
    char **list;
    char *name;
    int i, listws, listed, listmo, listte, listsz;

    listws = sizeof(_std_ws_curve) / sizeof(_std_ws_curve[0]);
    listed = sizeof(_std_ed_curve) / sizeof(_std_ed_curve[0]);
    listmo = sizeof(_std_mo_curve) / sizeof(_std_mo_curve[0]);
    listte = sizeof(_std_te_curve) / sizeof(_std_te_curve[0]);
    listsz = listws + listed + listmo + listte;
    list = (char **)malloc((listsz+1) * sizeof(char *));
    assert (list != NULL);
    for (i = 0; i < listws; i++) {
        name = (char *)malloc(sizeof(char)*strlen(_std_ws_curve[i].name)+1);
        strcpy(name, _std_ws_curve[i].name);
        list[i] = name;
    }
    for (i = 0; i < listed; i++) {
        name = (char *)malloc(sizeof(char)*strlen(_std_ed_curve[i].name)+1);
        strcpy(name, _std_ed_curve[i].name);
        list[i+listws] = name;
    }
    for (i = 0; i < listmo; i++) {
        name = (char *)malloc(sizeof(char)*strlen(_std_mo_curve[i].name)+1);
        strcpy(name, _std_mo_curve[i].name);
        list[i+listws+listed] = name;
    }
    for (i = 0; i < listte; i++) {
        name = (char *)malloc(sizeof(char)*strlen(_std_te_curve[i].name)+1);
        strcpy(name, _std_te_curve[i].name);
        list[i+listws+listed+listmo] = name;
    }
    list[listsz] = (char *)NULL;
    return list;
}
