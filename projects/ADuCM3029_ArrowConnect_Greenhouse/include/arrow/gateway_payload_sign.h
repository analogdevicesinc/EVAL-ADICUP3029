#if !defined(_ARROW_GATEWAY_PAYLOAD_SIGN_H_)
#define _ARROW_GATEWAY_PAYLOAD_SIGN_H_

//char gate_sign[128];
//char can_par_req[] = "key1=Value 1\nkey2=Value 2";
//gateway_payload_sign(gate_sign,
//                     "05c2d78dee6798025e6e3f83f79256914b7c3664",
//                     "update-configuration",
//                     0,
//                     can_par_req,
//                     "1");

int gateway_payload_sign(char *signature,
                         const char *hid,
                         const char *name,
                         int encrypted,
                         const char *canParString,
                         const char *signatureVersion);

#endif // _ARROW_GATEWAY_PAYLOAD_SIGN_H_
