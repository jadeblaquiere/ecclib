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

package main

import (
	"../shared"
	"flag"
	"fmt"
	"github.com/jadeblaquiere/ecclib/ecgo"
	"os"
)

func main() {

	// HANDLE FLAGS

	listcurves := flag.Bool("list-curves", false, "list the available curves and exit")
	curve := flag.String("curve", "secp256k1", "elliptic curve to use for key exchange")
	flag.Parse()

	if *listcurves {
		clist := ecgo.CurveNames()
		for _, cv := range clist {
			fmt.Println(cv)
		}
		os.Exit(0)
	}

	// FIND THE DESIRED/DEFAULT CURVE

	cv := ecgo.NamedCurve(*curve)
	if cv == nil {
		shared.ExitWMessage(1, "<Error>: Curve not found\n")
	}

	// GENERATE A RANDOM SCALAR (OF ORDER SAME AS CURVE)

	privkey := ecgo.NewFieldElementURandom(cv.GetAttr("n"))

	// EXPORT SCALAR VALUE, CURVE AS PRIVATE KEY

	der, err := shared.ECDHPrivkeyToDER(privkey.AsInt(), cv)
	shared.ExitOnError(err, "<Error>: unable to export DER binary format\n")

	shared.WriteB64Wrapped("ECDH PRIVATE KEY", der)
}
