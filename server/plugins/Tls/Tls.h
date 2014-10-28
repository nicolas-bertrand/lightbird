#ifndef TLS_H
# define TLS_H

// Windows does not define ssize_t
# ifdef Q_OS_WIN
#  define ssize_t qint64
# endif // Q_OS_WIN

#include <errno.h>
#include <gnutls/gnutls.h>
#include <gnutls/abstract.h>
#include <gnutls/crypto.h>
#include <gnutls/x509.h>
#include "LightBird.h"

// gnutls_session_t needs to be fully defined for Q_DECLARE_METATYPE, but gnutls_session_int is not a public struct,
// so we declare a fake one (which does not affect the size of gnutls_session_t).
struct gnutls_session_int { };
Q_DECLARE_METATYPE(gnutls_session_t)

#endif // TLS_H
