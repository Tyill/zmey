

//let date = dateFormat(d, 'yyyy-mm-dd hh:ii:ss');
export 
function dateFormat(date : Date, format : string) {
  let yyyy = date.getFullYear().toString();
  format = format.replace(/yyyy/g, yyyy)
  let mm = (date.getMonth()+1).toString(); 
  format = format.replace(/mm/g, (mm[1]?mm:"0"+mm[0]));
  let dd  = date.getDate().toString();
  format = format.replace(/dd/g, (dd[1]?dd:"0"+dd[0]));
  let hh = date.getHours().toString();
  format = format.replace(/hh/g, (hh[1]?hh:"0"+hh[0]));
  let ii = date.getMinutes().toString();
  format = format.replace(/ii/g, (ii[1]?ii:"0"+ii[0]));
  let ss  = date.getSeconds().toString();
  format = format.replace(/ss/g, (ss[1]?ss:"0"+ss[0]));
  let ms  = date.getMilliseconds().toString();
  if (!ms[2]) ms ="0" + ms;
  if (!ms[1]) ms ="00" + ms;
  format = format.replace(/ms/g, ms);
  return format;
};