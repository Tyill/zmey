/* eslint-disable no-undef */
module.exports = {
  entry: './tsx/app.tsx',  
  output: {
    publicPath: 'js/',
    path: __dirname + '/js/',
    filename: 'bundle.js'
  },
  mode: 'production',
    module: {
    rules: [
      { test: /\.css$/, loader: 'style-loader!css-loader'},
      { test: /\.tsx?$/, exclude: /(node_modules)/, use: 'babel-loader' }
    ],    
  },  
}