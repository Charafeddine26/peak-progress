// Generate a self-signed certificate using only Node.js built-ins
const { generateKeyPairSync, createSign, randomBytes } = require('crypto');
const fs = require('fs');

// Generate RSA key pair
const { publicKey, privateKey } = generateKeyPairSync('rsa', {
  modulusLength: 2048,
  publicKeyEncoding: { type: 'spki', format: 'pem' },
  privateKeyEncoding: { type: 'pkcs8', format: 'pem' },
});

// Write key
fs.writeFileSync('key.pem', privateKey);

// Build a minimal self-signed X.509 v3 certificate in DER, then PEM-encode it
// This is a simplified but functional approach using ASN.1/DER encoding

function derLength(len) {
  if (len < 128) return Buffer.from([len]);
  if (len < 256) return Buffer.from([0x81, len]);
  return Buffer.from([0x82, (len >> 8) & 0xff, len & 0xff]);
}

function derSequence(...items) {
  const body = Buffer.concat(items);
  return Buffer.concat([Buffer.from([0x30]), derLength(body.length), body]);
}

function derSet(...items) {
  const body = Buffer.concat(items);
  return Buffer.concat([Buffer.from([0x31]), derLength(body.length), body]);
}

function derOID(oid) {
  const parts = oid.split('.').map(Number);
  const encoded = [40 * parts[0] + parts[1]];
  for (let i = 2; i < parts.length; i++) {
    let val = parts[i];
    if (val >= 128) {
      const bytes = [];
      bytes.unshift(val & 0x7f);
      val >>= 7;
      while (val > 0) {
        bytes.unshift((val & 0x7f) | 0x80);
        val >>= 7;
      }
      encoded.push(...bytes);
    } else {
      encoded.push(val);
    }
  }
  const buf = Buffer.from(encoded);
  return Buffer.concat([Buffer.from([0x06]), derLength(buf.length), buf]);
}

function derUTF8String(s) {
  const buf = Buffer.from(s, 'utf8');
  return Buffer.concat([Buffer.from([0x0c]), derLength(buf.length), buf]);
}

function derPrintableString(s) {
  const buf = Buffer.from(s, 'ascii');
  return Buffer.concat([Buffer.from([0x13]), derLength(buf.length), buf]);
}

function derInteger(n) {
  if (Buffer.isBuffer(n)) {
    // ensure positive (add leading 0 if high bit set)
    if (n[0] & 0x80) n = Buffer.concat([Buffer.from([0]), n]);
    return Buffer.concat([Buffer.from([0x02]), derLength(n.length), n]);
  }
  if (n < 128) return Buffer.from([0x02, 0x01, n]);
  const bytes = [];
  let v = n;
  while (v > 0) { bytes.unshift(v & 0xff); v >>= 8; }
  if (bytes[0] & 0x80) bytes.unshift(0);
  return Buffer.concat([Buffer.from([0x02]), derLength(bytes.length), Buffer.from(bytes)]);
}

function derBitString(buf) {
  // 0 unused bits
  const inner = Buffer.concat([Buffer.from([0x00]), buf]);
  return Buffer.concat([Buffer.from([0x03]), derLength(inner.length), inner]);
}

function derUTCTime(date) {
  const s = date.toISOString().replace(/[-:T]/g, '').slice(2, 14) + 'Z';
  const buf = Buffer.from(s, 'ascii');
  return Buffer.concat([Buffer.from([0x17]), derLength(buf.length), buf]);
}

function derExplicit(tag, content) {
  return Buffer.concat([Buffer.from([0xa0 | tag]), derLength(content.length), content]);
}

// Parse the PEM public key to get the raw SPKI bytes
const spkiB64 = publicKey.replace(/-----[^-]+-----/g, '').replace(/\s/g, '');
const spkiDer = Buffer.from(spkiB64, 'base64');

// Build TBS (To Be Signed) Certificate
const serialNumber = derInteger(randomBytes(8));
const sigAlgOID = derSequence(derOID('1.2.840.113549.1.1.11'), Buffer.from([0x05, 0x00])); // SHA-256 with RSA

// Issuer/Subject: CN=PeakProgress
const cn = derSequence(derOID('2.5.4.3'), derUTF8String('PeakProgress'));
const name = derSequence(derSet(cn));

const notBefore = derUTCTime(new Date());
const notAfter = derUTCTime(new Date(Date.now() + 365 * 24 * 60 * 60 * 1000));
const validity = derSequence(notBefore, notAfter);

const tbsCertificate = derSequence(
  derExplicit(0, derInteger(2)), // v3
  serialNumber,
  sigAlgOID,
  name,      // issuer
  validity,
  name,      // subject (self-signed)
  spkiDer,   // subject public key info
);

// Sign
const sign = createSign('SHA256');
sign.update(tbsCertificate);
const signature = sign.sign(privateKey);

// Full certificate
const cert = derSequence(
  tbsCertificate,
  sigAlgOID,
  derBitString(signature),
);

// PEM encode
const certPem = '-----BEGIN CERTIFICATE-----\n' +
  cert.toString('base64').match(/.{1,64}/g).join('\n') +
  '\n-----END CERTIFICATE-----\n';

fs.writeFileSync('cert.pem', certPem);
console.log('✅ Generated cert.pem and key.pem');
