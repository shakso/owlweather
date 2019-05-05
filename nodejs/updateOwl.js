onst axios = require('axios');

axios.get('https://api.darksky.net/forecast/*APIKEYHERE*/51.077840,1.165460')
  .then(response => {
        var tl = ((response.data.daily.data[0].temperatureHigh)-32)*(5/9);
        var rl = response.data.daily.data[0].precipProbability*100;
        var tr = ((response.data.daily.data[1].temperatureHigh)-32)*(5/9);
        var rr = response.data.daily.data[1].precipProbability*100;

    console.log(tl + '-' +rl + '-' +tr + '-' +rr);

        tl=((parseInt(tl)+5)*-9.875)+515;
        rl=(parseInt(rl)*4.45)+125;
        rr=(parseInt(rr)*-4.15)+530;
        tr=((parseInt(tr)+5)*10.75)+120;

        axios.get('http://x.x.x.x/?tl=' + tl + '&rl=' + rl + '&tr=' + tr + '&rr=' + rr);
  })
  .catch(error => {
    console.log(error);
  });
