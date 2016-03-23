package com.matferib.Tabuleiro;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;

public class SelecaoActivity extends Activity implements View.OnClickListener {
  // Mensagem de comunicacao entre atividades.
  public final static String NOME = "com.matferib.Tabuleiro.NOME";
  public final static String ENDERECO = "com.matferib.Tabuleiro.ENDERECO";
  public final static String SERVIDOR = "com.matferib.Tabuleiro.SERVIDOR";
  public final static String MAPEAMENTO_SOMBRAS = "com.matferib.Tabuleiro.MAPEAMENTO_SOMBRAS";
  public final static String LUZ_POR_PIXEL = "com.matferib.Tabuleiro.LUZ_POR_PIXEL";

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.janela_conexao);
    // Pega os campos do XML.
    id_ = (EditText)findViewById(R.id.texto_id_jogador);
    id_.setText(android.os.Build.MODEL);
    endereco_ = (EditText)findViewById(R.id.texto_endereco_ou_ip);
    botao_ = (Button)findViewById(R.id.botao_conectar);
    botao_.setOnClickListener(this);
    botaoServidor_ = (Button)findViewById(R.id.botao_abrir_servidor);
    botaoServidor_.setOnClickListener(this);
  }

  @Override
  protected void onPause() {
    super.onPause();
  }

  @Override
  protected void onResume() {
    super.onResume();
  }

  @Override
  public void onClick(View v) {
    Intent intencao = new Intent(this, TabuleiroActivity.class);
    if (v == botaoServidor_) {
      // Abre a activity do tabuleiro.
      intencao.putExtra(SERVIDOR, endereco_.getText().toString());
    } else {
      // Abre a activity do tabuleiro.
      intencao.putExtra(NOME, id_.getText().toString());
      intencao.putExtra(ENDERECO, endereco_.getText().toString());
    }
    intencao.putExtra(MAPEAMENTO_SOMBRAS, ((CheckBox)findViewById(R.id.checkbox_mapeamento_sombras)).isChecked());
    intencao.putExtra(LUZ_POR_PIXEL, ((CheckBox)findViewById(R.id.checkbox_luz_por_pixel)).isChecked());
    startActivity(intencao);
  }

  // Membros.
  private EditText id_ = null;
  private EditText endereco_ = null;
  private Button botao_ = null, botaoServidor_ = null;
}
