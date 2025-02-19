
#include <exception>

#include <string.h>

#include "scitokens.h"
#include "scitokens_internal.h"

SciTokenKey scitoken_key_create(const char *key_id, const char *alg, const char *public_contents, const char *private_contents, char **err_msg) {
    if (key_id == nullptr) {
        if (err_msg) {*err_msg = strdup("Key ID cannot be NULL.");}
        return nullptr;
    }
    if (alg == nullptr) {
        if (err_msg) {*err_msg = strdup("Algorithm cannot be NULL.");}
        return nullptr;
    }
    if (public_contents == nullptr) {
        if (err_msg) {*err_msg = strdup("Public key contents cannot be NULL.");}
        return nullptr;
    }
    if (private_contents == nullptr) {
        if (err_msg) {*err_msg = strdup("Private key contents cannot be NULL.");}
        return nullptr;
    }
    return new scitokens::SciTokenKey(key_id, alg, public_contents, private_contents);
}

void scitoken_key_destroy(SciTokenKey token) {
    scitokens::SciTokenKey *real_token = reinterpret_cast<scitokens::SciTokenKey*>(token);
    delete real_token;
}

SciToken scitoken_create(SciTokenKey private_key) {
    scitokens::SciTokenKey *key = reinterpret_cast<scitokens::SciTokenKey*>(private_key);
    return new scitokens::SciToken(*key);
}

void scitoken_destroy(SciToken token) {
    scitokens::SciToken *real_token = reinterpret_cast<scitokens::SciToken*>(token);
    delete real_token;
}

int scitoken_set_claim_string(SciToken token, const char *key, const char *value, char **err_msg) {
    scitokens::SciToken *real_token = reinterpret_cast<scitokens::SciToken*>(token);
    if (real_token == nullptr) {
        if (err_msg) {*err_msg = strdup("Token passed is not initialized.");}
        return -1;
    }
    if (key == nullptr) {
        if (err_msg) {*err_msg = strdup("Claim key passed is not initialized.");}
        return -1;
    }
    if (value == nullptr) {
        if (err_msg) {*err_msg = strdup("Claim value passed is not initialized.");}
        return -1;
    }
    try {
        real_token->set_claim(key, jwt::claim(std::string(value)));
    } catch (std::exception &exc) {
        if (err_msg) {
            *err_msg = strdup(exc.what());
        }
        return -1;
    }
    return 0;
}


void scitoken_set_serialize_profile(SciToken token, SciTokenProfile profile) {
    scitoken_set_serialize_mode(token, profile);
}


void scitoken_set_serialize_mode(SciToken token, SciTokenProfile profile) {
    scitokens::SciToken *real_token = reinterpret_cast<scitokens::SciToken*>(token);
    if (real_token == nullptr) {return;}

    real_token->set_serialize_mode(static_cast<scitokens::SciToken::Profile>(profile));
}


void scitoken_set_deserialize_profile(SciToken token, SciTokenProfile profile) {
    scitokens::SciToken *real_token = reinterpret_cast<scitokens::SciToken*>(token);
    if (real_token == nullptr) {return;}

    real_token->set_deserialize_mode(static_cast<scitokens::SciToken::Profile>(profile));
}


int scitoken_get_claim_string(const SciToken token, const char *key, char **value, char **err_msg) {
    scitokens::SciToken *real_token = reinterpret_cast<scitokens::SciToken*>(token);
    std::string claim_str;
    try {
        claim_str = real_token->get_claim_string(key);
    } catch (std::exception &exc) {
        if (err_msg) {
            *err_msg = strdup(exc.what());
        }
        return -1;
    }
    *value = strdup(claim_str.c_str());
    return 0;
}


int scitoken_set_claim_string_list(const SciToken token, const char *key,
    const char **value, char **err_msg)
{
    auto real_token = reinterpret_cast<scitokens::SciToken*>(token);
    if (real_token == nullptr) {
        if (err_msg) *err_msg = strdup("NULL scitoken passed to scitoken_get_claim_string_list");
        return -1;
    }
    std::vector<std::string> claim_list;
    int idx = 0;
    while (value[idx++]) {}
    claim_list.reserve(idx);

    idx = 0;
    while (value[idx++]) {
        claim_list.emplace_back(value[idx-1]);
    }
    real_token->set_claim_list(key, claim_list);

    return 0;
}


int scitoken_get_claim_string_list(const SciToken token, const char *key, char ***value, char **err_msg) {
    auto real_token = reinterpret_cast<scitokens::SciToken*>(token);
    if (real_token == nullptr) {
        if (err_msg) *err_msg = strdup("NULL scitoken passed to scitoken_get_claim_string_list");
        return -1;
    }
    std::vector<std::string> claim_list;
    try {
        claim_list = real_token->get_claim_list(key);
    } catch (std::exception &exc) {
        if (err_msg) {*err_msg = strdup(exc.what());}
        return -1;
    }
    auto claim_list_c = static_cast<char **>(malloc(sizeof(char *) * (claim_list.size() + 1)));
    claim_list_c[claim_list.size()] = nullptr;
    int idx = 0;
    for (const auto &entry : claim_list) {
        claim_list_c[idx] = strdup(entry.c_str());
        if (!claim_list_c[idx]) {
            scitoken_free_string_list(claim_list_c);
            if (err_msg) {*err_msg = strdup("Failed to create a copy of string entry in list");}
            return -1;
        }
        idx++;
    }
    *value = claim_list_c;
    return 0;
}


void scitoken_free_string_list(char **value) {
    int idx = 0;
    do {
        free(value[idx++]);
    } while (value[idx]);
    free(value);
}


int scitoken_get_expiration(const SciToken token, long long *expiry, char **err_msg) {
    scitokens::SciToken *real_token = reinterpret_cast<scitokens::SciToken*>(token);
    if (!real_token->has_claim("exp")) {
        *expiry = -1;
        return 0;
    }

    long long result;
    try {
        result = real_token->get_claim("exp").as_int();
    } catch (std::exception &exc) {
        if (err_msg) {
            *err_msg = strdup(exc.what());
        }
        return -1;
    }
    *expiry = result;
    return 0;
}


void scitoken_set_lifetime(SciToken token, int lifetime) {
    if (token == nullptr) {return;}
    scitokens::SciToken *real_token = reinterpret_cast<scitokens::SciToken*>(token);
    real_token->set_lifetime(lifetime);
}


int scitoken_serialize(const SciToken token, char **value, char **err_msg) {
    if (value == nullptr) {
        if (err_msg) {*err_msg = strdup("Output variable not provided");}
        return -1;
    }
    scitokens::SciToken *real_token = reinterpret_cast<scitokens::SciToken*>(token);
    try {
        std::string serialized = real_token->serialize();
        *value = strdup(serialized.c_str());
    } catch (std::exception &exc) {
        if (err_msg) {
            *err_msg = strdup(exc.what());
        }
        return -1;
    }
    return 0;
}

int scitoken_deserialize(const char *value, SciToken *token, char const* const* allowed_issuers, char **err_msg) {
    if (value == nullptr) {
        if (err_msg) {*err_msg = strdup("Token may not be NULL");}
        return -1;
    }
    if (token == nullptr) {
        if (err_msg) {*err_msg = strdup("Output token not provided");}
        return -1;
    }

    scitokens::SciTokenKey key;
    scitokens::SciToken *real_token = new scitokens::SciToken(key);

    int retval = scitoken_deserialize_v2(value, reinterpret_cast<SciToken>(real_token), allowed_issuers, err_msg);
    if (retval) {
        delete real_token;
    } else {
        *token = real_token;
    }
    return retval;
}

int scitoken_deserialize_v2(const char *value, SciToken token, char const* const* allowed_issuers, char **err_msg) {
    scitokens::SciToken *real_token = reinterpret_cast<scitokens::SciToken*>(token);

    std::vector<std::string> allowed_issuers_vec;
    if (allowed_issuers != nullptr) {
        for (int idx=0; allowed_issuers[idx]; idx++) {
            allowed_issuers_vec.push_back(allowed_issuers[idx]);
        }
    }

    try {
        real_token->deserialize(value, allowed_issuers_vec);
    } catch (std::exception &exc) {
        if (err_msg) {
            *err_msg = strdup(exc.what());
        }
        return -1;
    }
    return 0;
}

int scitoken_store_public_ec_key(const char *issuer, const char *keyid, const char *key, char **err_msg)
{
    bool success;
    try {
        success = scitokens::Validator::store_public_ec_key(issuer, keyid, key);
    } catch (std::exception &exc) {
        if (err_msg) {
            *err_msg = strdup(exc.what());
        }
        return -1;
    }

    return success ? 0 : -1;
}

Validator validator_create() {
    return new Validator();
}


void validator_destroy(Validator validator) {
    scitokens::Validator *real_validator =
        reinterpret_cast<scitokens::Validator*>(validator);
    delete real_validator;
}


void validator_set_token_profile(Validator validator, SciTokenProfile profile) {
    if (validator == nullptr) {return;}
    auto real_validator = reinterpret_cast<scitokens::Validator*>(validator);
    real_validator->set_validate_profile(static_cast<scitokens::SciToken::Profile>(profile));
}

int validator_add(Validator validator, const char *claim, StringValidatorFunction validator_func, char **err_msg) {
    if (validator == nullptr) {
        if (err_msg) {*err_msg = strdup("Validator may not be a null pointer");}
        return -1;
    }
    auto real_validator = reinterpret_cast<scitokens::Validator*>(validator);
    if (claim == nullptr) {
        if (err_msg) {*err_msg = strdup("Claim name may not be a null pointer");}
        return -1;
    }
    if (validator_func == nullptr) {
        if (err_msg) {*err_msg = strdup("Validator function may not be a null pointer");}
        return -1;
    }
    real_validator->add_string_validator(claim, validator_func);
    return 0;
}

int validator_add_critical_claims(Validator validator, const char **claims, char **err_msg) {
    if (validator == nullptr) {
        if (err_msg) {*err_msg = strdup("Validator may not be a null pointer");}
        return -1;
    }
    auto real_validator = reinterpret_cast<scitokens::Validator*>(validator);
    if (claims == nullptr) {
        if (err_msg) {*err_msg = strdup("Claim list may not be a null pointer");}
        return -1;
    }
    std::vector<std::string> claims_vec;
    for (int idx=0; claims[idx]; idx++) {
        claims_vec.push_back(claims[idx]);
    }
    real_validator->add_critical_claims(claims_vec);
    return 0;
}


int validator_validate(Validator validator, SciToken scitoken, char **err_msg) {
    if (validator == nullptr) {
        if (err_msg) {*err_msg = strdup("Validator may not be a null pointer");}
        return -1;
    }
    auto real_validator = reinterpret_cast<scitokens::Validator*>(validator);
    if (scitoken == nullptr) {
        if (err_msg) {*err_msg = strdup("SciToken may not be a null pointer");}
        return -1;
    }
    auto real_scitoken = reinterpret_cast<scitokens::SciToken*>(scitoken);

    try {
        real_validator->verify(*real_scitoken);
    } catch (std::exception &exc) {
        if (err_msg) {*err_msg = strdup(exc.what());}
        return -1;
    }
    return 0;
}


Enforcer enforcer_create(const char *issuer, const char **audience_list, char **err_msg) {
    if (issuer == nullptr) {
        if (err_msg) {*err_msg = strdup("Issuer may not be a null pointer");}
        return nullptr;
    }
    std::vector<std::string> aud_list;
    if (audience_list != nullptr) {
        for (int idx=0; audience_list[idx]; idx++) {
            aud_list.push_back(audience_list[idx]);
        }
    }

    return new scitokens::Enforcer(issuer, aud_list);
}


void enforcer_destroy(Enforcer enf) {
    if (enf == nullptr) {
        return;
    }
    auto real_enf = reinterpret_cast<scitokens::Enforcer*>(enf);
    delete real_enf;
}

void enforcer_acl_free(Acl *acls) {
    for (int idx=0; acls[idx].authz == nullptr && acls[idx].resource == nullptr; idx++) {
        free(const_cast<char *>(acls[idx].authz));
        free(const_cast<char *>(acls[idx].resource));
    }
    free(acls);
}


void enforcer_set_validate_profile(Enforcer enf, SciTokenProfile profile) {
    if (enf == nullptr) {return;}

    auto real_enf = reinterpret_cast<scitokens::Enforcer*>(enf);
    real_enf->set_validate_profile(static_cast<scitokens::SciToken::Profile>(profile));
}


int enforcer_generate_acls(const Enforcer enf, const SciToken scitoken, Acl **acls, char **err_msg) {
    if (enf == nullptr) {
        if (err_msg) {*err_msg = strdup("Enforcer may not be a null pointer");}
        return -1;
    }
    auto real_enf = reinterpret_cast<scitokens::Enforcer*>(enf);
    if (scitoken == nullptr) {
        if (err_msg) {*err_msg = strdup("SciToken may not be a null pointer");}
        return -1;
    }
    auto real_scitoken = reinterpret_cast<scitokens::SciToken*>(scitoken);

    scitokens::Enforcer::AclsList acls_list;
    try {
         acls_list = real_enf->generate_acls(*real_scitoken);
    } catch (std::exception &exc) {
        if (err_msg) {*err_msg = strdup(exc.what());}
        return -1;
    }
    Acl *acl_result = static_cast<Acl*>(malloc((acls_list.size() + 1)*sizeof(Acl)));
    size_t idx = 0;
    for (const auto &acl : acls_list) {
        acl_result[idx].authz = strdup(acl.first.c_str());
        acl_result[idx].resource = strdup(acl.second.c_str());
        if (acl_result[idx].authz == nullptr) {
            enforcer_acl_free(acl_result);
            if (err_msg) {*err_msg = strdup("ACL was generated without an authorization set.");}
            return -1;
        }
        if (acl_result[idx].resource == nullptr) {
            enforcer_acl_free(acl_result);
            if (err_msg) {*err_msg = strdup("ACL was generated without a resource set.");}
            return -1;
        }
        idx++;
    }
    acl_result[idx].authz = nullptr;
    acl_result[idx].resource = nullptr;
    *acls = acl_result;
    return 0;
}


int enforcer_test(const Enforcer enf, const SciToken scitoken, const Acl *acl, char **err_msg) {
    if (enf == nullptr) {
        if (err_msg) {*err_msg = strdup("Enforcer may not be a null pointer");}
        return -1;
    }
    auto real_enf = reinterpret_cast<scitokens::Enforcer*>(enf);
    if (scitoken == nullptr) {
        if (err_msg) {*err_msg = strdup("SciToken may not be a null pointer");}
        return -1;
    }
    auto real_scitoken = reinterpret_cast<scitokens::SciToken*>(scitoken);
    if (acl == nullptr) {
        if (err_msg) {*err_msg = strdup("ACL may not be a null pointer");}
        return -1;
    }

    try {
        return real_enf->test(*real_scitoken, acl->authz, acl->resource) == true ? 0 : -1;
    } catch (std::exception &exc) {
        if (err_msg) {*err_msg = strdup(exc.what());}
        return -1;
    }
    return 0;
}
