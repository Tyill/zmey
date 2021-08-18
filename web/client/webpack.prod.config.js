/* eslint-disable no-undef */
module.exports = {
  entry: './client/app.tsx',  
  output: {
    publicPath: 'js/',
    path: __dirname + '/js/',
    filename: 'bundle.js'
  },
  resolve: {
    extensions: [".ts", ".tsx", ".css", ".less", ".js"],
    modules: ['node_modules']
  },
  mode: 'production',
  module: {
    rules: [
      { test: /\.css$/, loader: 'style-loader!css-loader'},
      { test: /\.less$/, loader: 'style-loader!css-loader!less-loader'},
      { test: /\.tsx?$/, exclude: /(node_modules)/, use: 'babel-loader' },
      { test: /\.(woff|woff2|ttf|eot)$/, use: 'file-loader?name=fonts/[name].[ext]!static' }
    ],    
  },  
}