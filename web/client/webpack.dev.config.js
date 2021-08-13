/* eslint-disable no-undef */
const webpack = require('webpack');

module.exports = {
  entry: [
    'webpack-dev-server/client/?http://localhost:8080',
    './client/app.tsx',
  ],  
  output: {
    publicPath: 'js/',
    path: __dirname + '/js/',
    filename: 'bundle.js'
  },
  devtool:  '#sourcemap',
  mode: 'development',
  resolve: {
    extensions: [".ts", ".tsx", '.css', ".js"],
    modules: ['node_modules']
  },
  module: {
    rules: [
      { test: /\.css$/, loader: 'style-loader!css-loader'},
      { test: /\.tsx?$/, exclude: /(node_modules)/, use: ['babel-loader'] },
      { test: /\.(woff|woff2|ttf|eot)$/, use: 'file-loader?name=fonts/[name].[ext]!static' }
    ]    
   },
  devServer: {
    hot: true,
    proxy: {
      '/': 'http://localhost:5000', 
    },   
  },  
  plugins: [new webpack.HotModuleReplacementPlugin()]   
}