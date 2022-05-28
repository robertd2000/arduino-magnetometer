const getData = async () => {
  const data = await fetch(
    'http://192.168.0.12:8080/get?magAccuracy&magX&magY',
    {
      headers: {
        'User-Agent':
          'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/102.0.5005.62 Safari/537.36',
        'Accept-Encoding': 'gzip, deflate',
        accept: '*/*',
        'Accept-Language': 'ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7',
      },
    }
  )
  const res = await data.json()
  return {
    magX: res.buffer.magX.buffer[0],
    magY: res.buffer.magY.buffer[0],
  }
}

document.addEventListener('DOMContentLoaded', () => {
  const { magX, magY } = getData()
  console.log(magX, magY)
})
