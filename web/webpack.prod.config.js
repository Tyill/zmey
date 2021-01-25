/* eslint-disable no-undef */
module.exports = {
  entry: './tsx/app.tsx',  
  output: {
    publicPath: 'js/',
    path: __dirname + '/js/',
    filename: 'bundle.js'
  },
  resolve: {
    extensions: [".ts", ".tsx", '.css', ".js"],
    modules: ['node_modules']
  },
  mode: 'production',
  module: {
    rules: [
      { test: /\.css$/, loader: 'style-loader!css-loader'},
      { test: /\.tsx?$/, exclude: /(node_modules)/, use: 'babel-loader' },
      { test: /\.(woff|woff2|ttf|eot)$/, use: 'file-loader?name=fonts/[name].[ext]!static' }
    ],    
  },  
}