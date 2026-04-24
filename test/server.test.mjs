import test from 'node:test';
import assert from 'node:assert/strict';
import { once } from 'node:events';
import http from 'node:http';
import { createRequire } from 'node:module';

const require = createRequire(import.meta.url);
const { createServer } = require('../server.js');

async function withServer(run) {
  const server = createServer();
  server.listen(0, '127.0.0.1');
  await once(server, 'listening');

  const { port } = server.address();
  const baseUrl = `http://127.0.0.1:${port}`;

  try {
    await run(baseUrl);
  } finally {
    server.close();
    await once(server, 'close');
  }
}

function request(baseUrl, requestPath) {
  return new Promise((resolve, reject) => {
    const url = new URL(baseUrl);
    const req = http.request(
      {
        host: url.hostname,
        port: url.port,
        path: requestPath,
        method: 'GET',
      },
      (res) => {
        let body = '';
        res.setEncoding('utf8');
        res.on('data', (chunk) => {
          body += chunk;
        });
        res.on('end', () => {
          resolve({ body, response: res });
        });
      }
    );

    req.on('error', reject);
    req.end();
  });
}

test('serves the index page for /', async () => {
  await withServer(async (baseUrl) => {
    const response = await fetch(`${baseUrl}/`);
    const body = await response.text();

    assert.equal(response.status, 200);
    assert.match(response.headers.get('content-type') ?? '', /text\/html/);
    assert.match(body, /laser harp/i);
  });
});

test('blocks directory traversal requests', async () => {
  await withServer(async (baseUrl) => {
    const { response, body } = await request(baseUrl, '/../package.json');

    assert.equal(response.statusCode, 403);
    assert.equal(body, 'Forbidden');
  });
});

test('returns 404 for missing files', async () => {
  await withServer(async (baseUrl) => {
    const response = await fetch(`${baseUrl}/missing-file.txt`);
    const body = await response.text();

    assert.equal(response.status, 404);
    assert.equal(body, 'Not Found');
  });
});
