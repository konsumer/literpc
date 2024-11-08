import { readFile } form 'node:fs/promises'

function exportDefs(defs) {
  const out = []
  return out.join('\n')
}

const [NAME, DEFS] = process.argv

if (!defs) {
  console.error(`Usage: ${NAME} <JSON_DEF_FILE>`)
  process.exit(1)
}

console.log(exportDefs(JSON.parse(await readFile(DEFS, 'utf8'))))