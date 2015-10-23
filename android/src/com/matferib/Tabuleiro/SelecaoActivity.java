package com.matferib.Tabuleiro;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;

public class SelecaoActivity extends Activity implements View.OnClickListener {
  // Mensagem de comunicacao entre atividades.
  public final static String NOME = "com.matferib.Tabuleiro.NOME";
  public final static String ENDERECO = "com.matferib.Tabuleiro.ENDERECO";
  public final static String SERVIDOR = "com.matferib.Tabuleiro.SERVIDOR";

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    LinearLayout grupo = new LinearLayout(this);
    //grupo.setWeightSum(1.0f);
    grupo.setOrientation(LinearLayout.VERTICAL);
    grupo.setGravity(android.view.Gravity.CENTER_HORIZONTAL);
    // Nome.
    id_ = new EditText(this);
    id_.setHint("identificador do jogador");
    id_.setText(android.os.Build.MODEL);
    id_.setLayoutParams(
        new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 0.0f));
    grupo.addView(id_);
    // Endereco.
    endereco_ = new EditText(this);
    endereco_.setHint("endereço ou IP (vazio para automático)");
    endereco_.setLayoutParams(
        new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 0.0f));
    grupo.addView(endereco_);

    // Botao.
    botao_ = new Button(this);
    botao_.setText("Conectar");
    botao_.setOnClickListener(this);
    botao_.setLayoutParams(
        new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 0.0f));
    grupo.addView(botao_);
    // Servidor;
    botaoServidor_ = new Button(this);
    botaoServidor_.setText("Abrir Servidor");
    botaoServidor_.setOnClickListener(this);
    botaoServidor_.setLayoutParams(
        new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 0.0f));
    grupo.addView(botaoServidor_);
    // Finaliza.
    setContentView(grupo);
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
    startActivity(intencao);
  }

  // Membros.
  private EditText id_ = null;
  private EditText endereco_ = null;
  private Button botao_ = null, botaoServidor_ = null;
}
