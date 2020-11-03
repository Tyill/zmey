/* eslint-disable no-undef */
module.exports = {
  entry: './tsx/app.tsx',  
  output: {
    publicPath: 'zmBackend/js/',
    path: __dirname + '/zmBackend/js/',
    filename: 'bundle.js'
  },
  resolve: {
    extensions: [".ts", ".tsx", ".js"]
  },
  mode: 'production',
  module: {
    rules: [
      { test: /\.css$/, loader: 'style-loader!css-loader'},
      { test: /\.tsx?$/, exclude: /(node_modules)/, use: 'babel-loader' }
    ],    
  },  
}